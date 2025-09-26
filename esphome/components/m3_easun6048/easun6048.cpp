#include "easun6048.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m3_easun6048 {

const std::string STATE_UNAVAILABLE("unavailable");
const std::string STATE_UNKNOWN("unknown");
const char *const TAG = "m3_easun6048";
const char *FRAME_CONTEXT_LABELS[FRAME_CONTEXT_COUNT] = {"set_frame", "config_frame", "status_frame"};

void Easun6048::setup() {
  for (int i = 0; i < FRAME_CONTEXT_COUNT; ++i) {
    this->rx_frame_context_[i].rx_frame = {};
  }
  switch (this->serial_mode_) {
    case MASTER:
      // Set up for master mode
      break;
    case SLAVE:
      // Set up for slave mode
      if (this->tx_pin_ != UINT8_MAX) {
        // Disable TX in slave mode since we rely on display controller to poll/query
        // the control unit
#if defined(USE_ARDUINO)
        // TODO: implement variants for ESP32 and ESP-IDF
        pinMode(this->tx_pin_, INPUT);
#endif
      }
      break;
  }
}

void Easun6048::dump_config() {
#define prefix "  "
  ESP_LOGCONFIG(TAG,
                "Easun6048:"
                "\n  serial mode: %s",
                this->serial_mode_ == MASTER ? "MASTER" : "SLAVE");
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR(prefix, "link_connected", this->link_connected_binary_sensor_);
#endif

  for (int i = 0; i < FRAME_CONTEXT_COUNT; ++i) {
    auto &frame_context = this->rx_frame_context_[i];
    for (auto entity : frame_context.entities) {
      entity->dump_config(prefix, FRAME_CONTEXT_LABELS[i]);
    }
#ifdef USE_TEXT_SENSOR
    if (frame_context.text_sensor) {
      ESP_LOGCONFIG(TAG, "%s%s '%s'", prefix, FRAME_CONTEXT_LABELS[i], frame_context.text_sensor->get_name().c_str());
    }
#endif
  }
}

void Easun6048::loop() {
  const uint32_t epoch = millis();
  int avail = this->available();
  if (avail) {
    this->rx_last_epoch_ = epoch;
    ESP_LOGV(TAG, "Available %d bytes", avail);
    // using local references
    byte *_rx_buffer_write_ = this->rx_buffer_write_;
    byte *_rx_buffer_read_ = this->rx_buffer_read_;

    int to_read = std::min(avail, (int) (this->rx_buffer_end_ - _rx_buffer_write_));
    this->read_array(_rx_buffer_write_, to_read);
    _rx_buffer_write_ += to_read;
    ESP_LOGV(TAG, "Read %d bytes, buffer now %d bytes", to_read, (int) (_rx_buffer_write_ - _rx_buffer_read_));
    switch (this->rx_frame_state_) {
      case RX_WAIT_HEADER:
      __rx_wait_header:
        while (_rx_buffer_read_ < _rx_buffer_write_) {
          if (*_rx_buffer_read_++ == FrameMarker::HEADER) {
            ESP_LOGV(TAG, "Found frame header");
            goto __rx_wait_data;
          }
        }
        // didn't find header, reset buffer
        goto __rx_reset;
      case RX_WAIT_DATA:
      __rx_wait_data:
        // _rx_buffer_read_ points to byte after header (i.e. length of payload)
        if (_rx_buffer_read_ == _rx_buffer_write_) {
          // need more data
          goto __rx_hold;
        }
        const int _payload_length = *_rx_buffer_read_;
        byte *const _rx_frame_end = _rx_buffer_read_ + _payload_length + 3;
        if (_rx_buffer_write_ < _rx_frame_end) {
          ESP_LOGV(TAG, "Need more data for full frame, have %d need %d", _rx_buffer_write_ - _rx_buffer_read_,
                   (_payload_length + 3));
          // need more data
          if (_rx_buffer_write_ == this->rx_buffer_end_) {
            // buffer full but incomplete frame, discard and restart looking for header
            ESP_LOGD(TAG, "Frame buffer overflow");
            goto __rx_reset;
          }
          goto __rx_hold;
        }
        if (*(_rx_buffer_read_ + _payload_length + 2) != FrameMarker::FOOTER) {
          ESP_LOGD(TAG, "Invalid end marker");
          // invalid end marker, restart looking for header
          goto __rx_wait_header;
        }
        if (GenericFrame::calc_checksum(_rx_buffer_read_) != *(_rx_buffer_read_ + _payload_length + 1)) {
          ESP_LOGD(TAG, "Invalid checksum");
          // invalid checksum, restart looking for header
          _rx_buffer_read_ = _rx_frame_end;
          goto __rx_wait_header;
        }

        // got a full frame: start parsing
        GenericFrame *frame = reinterpret_cast<GenericFrame *>(_rx_buffer_read_ - 1);
        const int _frame_length = _payload_length + 4;
        ESP_LOGV(TAG, "Got full frame: type=%u, payload length=%d", (unsigned int) frame->type, _payload_length);
        this->rx_last_frame_epoch_ = epoch;
#ifdef USE_BINARY_SENSOR
        if (this->link_connected_binary_sensor_) {
          this->link_connected_binary_sensor_->publish_state(true);
        }
#endif
        byte _context_idx = FRAME_TYPE_CONTEXT(frame->type);
        if (_context_idx < FRAME_CONTEXT_COUNT) {
          auto &frame_context = this->rx_frame_context_[_context_idx];
          if (memcmp(&frame_context.rx_frame, frame, _frame_length) != 0) {
            memcpy(&frame_context.rx_frame, frame, _frame_length);
            for (auto _entity : frame_context.entities) {
              _entity->parse(frame);
            }
#ifdef USE_TEXT_SENSOR
            if (frame_context.text_sensor) {
              frame_context.text_sensor->publish_state(format_hex_pretty(frame->raw, _frame_length, ':', false));
            }
#endif
          }
        } else {
          ESP_LOGD(TAG, "Unhandled frame type %u", (unsigned int) frame->type);
        }
        _rx_buffer_read_ = _rx_frame_end;
        // send any pending config requests
        if (!this->pending_config_requests_.empty()) {
          this->flush_config_requests_();
        }
        goto __rx_wait_header;
    }

  __rx_hold:
    this->rx_frame_state_ = RX_WAIT_DATA;
    this->rx_buffer_write_ = _rx_buffer_write_;
    this->rx_buffer_read_ = _rx_buffer_read_;
    return;

  __rx_reset:
    this->rx_frame_state_ = RX_WAIT_HEADER;
    this->rx_buffer_read_ = this->rx_buffer_write_ = this->rx_buffer_;
    return;

  } else if (this->rx_last_frame_epoch_ && (epoch - this->rx_last_frame_epoch_ > RX_TIMEOUT_MS)) {
    ESP_LOGW(TAG, "RX timeout");
    this->rx_last_frame_epoch_ = 0;
    this->pending_config_requests_.clear();
#ifdef USE_BINARY_SENSOR
    if (this->link_connected_binary_sensor_) {
      this->link_connected_binary_sensor_->publish_state(false);
    }
#endif
    for (int i = 0; i < FRAME_CONTEXT_COUNT; ++i) {
      auto &frame_context = this->rx_frame_context_[i];
      frame_context.rx_frame = {};
      for (auto _entity : frame_context.entities) {
        _entity->set_unavailable();
      }
#ifdef USE_TEXT_SENSOR
      if (frame_context.text_sensor) {
        frame_context.text_sensor->publish_state(STATE_UNAVAILABLE);
      }
#endif
    }
    this->rx_frame_state_ = RX_WAIT_HEADER;
    this->rx_buffer_read_ = this->rx_buffer_write_ = this->rx_buffer_;
  }
}

void Easun6048::update() {
  // Actually, when in SLAVE mode, this handler should never be invoked
  switch (this->serial_mode_) {
    case MASTER:
      // In master mode we poll the controller for config/status frames
      this->write_array({FrameMarker::HEADER, 1, FrameType::CONFIG, 3, FrameMarker::FOOTER});
      this->set_timeout(100, [this]() {
        this->write_array({FrameMarker::HEADER, 1, FrameType::STATUS, 4, FrameMarker::FOOTER});
      });
      break;
    case SLAVE:
      // In slave mode we don't do anything here, just wait for the display controller to poll us
      break;
  }
}

void Easun6048::send_config(byte offset, byte value) {
  auto &_rx_config_frame_ = this->rx_frame_context_[FRAME_TYPE_CONTEXT(CONFIG)].rx_frame;
  if (!_rx_config_frame_.header) {
    ESP_LOGW(TAG, "No config frame received yet, cannot send config");
    return;
  }
  int payload_size = _rx_config_frame_.payload_size;
  if (offset >= payload_size) {
    ESP_LOGW(TAG, "Invalid config offset %d (config frame payload size = %d)", offset, payload_size);
    return;
  }

  switch (this->serial_mode_) {
    case MASTER:
      // Allocate a buffer big enough to store the full received config frame
      // This is a bit overkill since it should have 10 bytes payload but..we'll adapt
      // to what's received
      Frame<sizeof(_rx_config_frame_) - 4> frame;
      memcpy(&frame, &_rx_config_frame_, sizeof(frame));
      // header, payload_size and footer already set from received frame
      frame.type = FrameType::SET;
      frame.payload[offset] = value;
      frame.payload[payload_size] = GenericFrame::calc_checksum(&frame.payload_size);
      this->write_array(frame.raw, payload_size + 4);
      break;
    case SLAVE:
      this->pending_config_requests_.push_back({offset, value});
      if ((millis() - this->rx_last_epoch_) < TX_SAFE_DELAY_MS) {
        this->flush_config_requests_();
      } else {
        ESP_LOGD(TAG, "Delaying config send to avoid collision with display transmission");
      }
      break;
  }
}

void Easun6048::flush_config_requests_() {
  // This method is only called when serial_mode is SLAVE
  // so that our transmission must be interleaved and not collide with the display
  // controller transmission. To achieve this we wait for a valid config
  // frame to be received from the controller and then respond with a SET
  // frame containing the requested changes.

#if defined(USE_ARDUINO) && defined(USE_ESP8266)
  // Set pin mode to output to enable transmission
  switch (this->tx_pin_) {
    case 1:  // UART0
      pinMode(1, FUNCTION_0);
      break;
    case 2:  // UART0/1
      pinMode(2, FUNCTION_4);
      break;
    case 15:  // UART0 SWAP
      pinMode(15, FUNCTION_4);
      break;
    default:
      // Assuming we're using a SoftwareSerial for ESP8266
      pinMode(this->tx_pin_, OUTPUT);
      break;
  }
    // TODO: implement variants for ESP32 and ESP-IDF
#endif  // defined(USE_ESP8266) && defined(USE_ARDUINO)

  // Allocate a buffer big enough to store the full frame received config frame
  // This is a bit overkill since it should have 10 bytes payload but..we'll adapt
  // to what's received
  auto &_rx_config_frame_ = this->rx_frame_context_[FRAME_TYPE_CONTEXT(CONFIG)].rx_frame;
  Frame<sizeof(_rx_config_frame_) - 4> frame;
  memcpy(&frame, &_rx_config_frame_, sizeof(frame));

  // header, payload_size and footer already set from received frame
  frame.type = FrameType::SET;
  const int payload_size = frame.payload_size;
  for (const auto &req : this->pending_config_requests_) {
    if (req.offset >= payload_size) {
      ESP_LOGW(TAG, "Invalid config offset %d (config frame payload size = %d)", req.offset, payload_size);
      continue;
    }
    frame.payload[req.offset] = req.value;
  }
  frame.payload[payload_size] = GenericFrame::calc_checksum(&frame.payload_size);
  this->write_array(frame.raw, payload_size + 4);
  this->pending_config_requests_.clear();

#if defined(USE_ARDUINO) && defined(USE_ESP8266)
  this->set_timeout(10 + payload_size, [this]() {
    // Reset pin mode to input after transmission completes
    // (1ms per byte at 9600 baud)
    pinMode(this->tx_pin_, INPUT);
  });
#endif
}

}  // namespace m3_easun6048
}  // namespace esphome
