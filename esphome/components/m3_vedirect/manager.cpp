#include "manager.h"
#include "esphome/core/log.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/components/uart/uart_component.h"
// #include <esphome/components/deep_sleep/deep_sleep_component.h>

#ifdef USE_ESP32
#include "esp_pm.h"
#include <esp_sleep.h>
#include <esp_wifi.h>
#endif

#include "factory.h"
#include "entity.h"
#include "hexframe.h"

namespace esphome {
namespace m3_vedirect {

static const char TAG[] = "m3_vedirect.%s";
static const char *const CHECKSUM_RECORD_NAME = "Checksum";

std::vector<Manager *> Manager::managers_;

void Manager::setup() {
  char *buf = new char[sizeof(TAG) + this->vedirect_id_.length()];
  sprintf(buf, TAG, this->vedirect_id_.c_str());
  this->logtag_ = buf;
  this->state_func_ = &Manager::state_idle_;
  this->buf_end_ = this->buf_ + sizeof(this->buf_);
  this->buf_write_ = this->buf_read_ = this->buf_;
  /*
  Experiment with low power modes
  esp_pm_config_esp32c3_t pm_config;
  esp_pm_get_configuration(&pm_config);
  pm_config.light_sleep_enable = true;
  esp_pm_configure(&pm_config);
  */
  Manager::managers_.push_back(this);
}

void Manager::loop() {
  uint32_t millis_ = millis();
  auto available = this->available();
  if (!available)
    if (this->connected_ && ((millis_ - this->millis_last_rx_) > VEDIRECT_TIMEOUT_MILLIS)) {
      this->on_disconnected_();
    }
  return;

  auto buf_available = this->buf_end_ - this->buf_write_;
  if (available > buf_available) {
    if (buf_available) {
      available = buf_available;
    } else {
      ESP_LOGE(this->logtag_, "global overflow: resetting buffers");
      this->buf_read_ = this->buf_write_ = this->buf_;
      buf_available = sizeof(this->buf_);
      this->state_func_ = &Manager::state_idle_;
      if (available > sizeof(this->buf_))
        available = sizeof(this->buf_);
    }
  }

  if (this->read_array(this->buf_write_, available)) {
    this->millis_last_rx_ = millis_;
    this->buf_write_ += available;
  loop_state:
#ifdef VEDIRECT_DEBUG_STATE
    ESP_LOGV(this->logtag_, "STATE(enter): read=%d write=%d", this->buf_read_ - this->buf_,
             this->buf_write_ - this->buf_);
#endif

    switch ((this->*state_func_)()) {
      case StateAction::Flush:
#ifdef VEDIRECT_DEBUG_STATE
        ESP_LOGV(this->logtag_, "STATE(Flush): read=%d write=%d", this->buf_read_ - this->buf_,
                 this->buf_write_ - this->buf_);
#endif
        this->buf_read_ = this->buf_write_ = this->buf_;
        break;

      case StateAction::Continue:
#ifdef VEDIRECT_DEBUG_STATE
        ESP_LOGV(this->logtag_, "STATE(Continue): read=%d write=%d", this->buf_read_ - this->buf_,
                 this->buf_write_ - this->buf_);
#endif
        goto loop_state;

      case StateAction::Hold:
#ifdef VEDIRECT_DEBUG_STATE
        ESP_LOGV(this->logtag_, "STATE(Hold): read=%d write=%d", this->buf_read_ - this->buf_,
                 this->buf_write_ - this->buf_);
#endif
        break;
    }
  }
}

void Manager::dump_config() { ESP_LOGCONFIG(this->logtag_, "VEDirect:"); }

void Manager::send_hexframe(const HexTxFrame &hex_frame) {
  auto &raw_frame = hex_frame.encode();
  this->write_array((const uint8_t *) raw_frame.c_str(), raw_frame.length());
  ESP_LOGD(this->logtag_, "HEX FRAME: sent %s", raw_frame.c_str());
}

/*static*/ void Manager::send_hexframe(const std::string &vedirect_id, const std::string &payload) {
  if (vedirect_id.empty()) {
    managers_.front()->send_hexframe(HexTxFrame(payload.c_str()));
  } else {
    for (auto manager : managers_) {
      if (manager->vedirect_id_ == vedirect_id) {
        manager->send_hexframe(HexTxFrame(payload.c_str()));
        break;
      }
    }
  }
}

Manager::StateAction Manager::state_idle_() {
  while (this->buf_read_ < this->buf_write_) {
    switch (*this->buf_read_) {
      case '\n':  // start of TEXT FRAME
        this->checksum_ += '\n';
        this->record_count_ = 0;
        this->name_[0] = reinterpret_cast<char *>(++this->buf_read_);
        this->buf_name_end_ = this->buf_read_ + VEDIRECT_NAME_LEN;
        this->state_func_ = &Manager::state_name_;
        return StateAction::Continue;
      case '\r':  // pre-start of TEXT FRAME
        ++this->buf_read_;
        this->checksum_ = '\r';
        break;
      case ':':  // HEX FRAME
        return this->enter_state_hex_();
      default:
        ++this->buf_read_;
    }
  }
  // at this stage, we're still in idle and have
  // 'consumed' all the buffered data so
  // we can safely reset the pointers so that
  // the next uart read will fill from the start
  return StateAction::Flush;
}
Manager::StateAction Manager::state_name_() {
  while (this->buf_read_ < this->buf_write_) {
    switch (*this->buf_read_) {
      case '\t':  // end of name
        this->checksum_ += '\t';
        *this->buf_read_ = 0;
        if (strcmp(this->name_[this->record_count_], CHECKSUM_RECORD_NAME)) {
          this->value_[this->record_count_] = reinterpret_cast<char *>(++this->buf_read_);
          this->buf_value_end_ = this->buf_read_ + VEDIRECT_VALUE_LEN;
          this->state_func_ = &Manager::state_value_;
        } else {  // the Checksum record indicates a EOF
          ++this->buf_read_;
          this->state_func_ = &Manager::state_checksum_;
        }
        return StateAction::Continue;
      case ':':  // HEX FRAME
        return this->enter_state_hex_();
      default:
        this->checksum_ += *this->buf_read_;
        if (++this->buf_read_ >= this->buf_name_end_) {
          ESP_LOGE(this->logtag_, "TEXT FRAME: overflow NAME");
          this->state_func_ = &Manager::state_idle_;
          return StateAction::Continue;
        }
    }
  }
  return StateAction::Hold;
}
Manager::StateAction Manager::state_value_() {
  while (this->buf_read_ < this->buf_write_) {
    switch (*this->buf_read_) {
      case '\n':  // start of next record
        this->checksum_ += '\n';
        *this->buf_read_ = 0;
        this->name_[++this->record_count_] = reinterpret_cast<char *>(++this->buf_read_);
        this->buf_name_end_ = this->buf_read_ + VEDIRECT_NAME_LEN;
        this->state_func_ = &Manager::state_name_;
        return StateAction::Continue;
      case '\r':  // pre-start of next record
        this->checksum_ += '\r';
        *this->buf_read_ = 0;
        ++this->buf_read_;
        break;
      case ':':  // HEX FRAME
        return this->enter_state_hex_();
      default:
        this->checksum_ += *this->buf_read_;
        if (++this->buf_read_ >= this->buf_value_end_) {
          ESP_LOGE(this->logtag_, "TEXT FRAME: overflow VALUE");
          this->state_func_ = &Manager::state_idle_;
          return StateAction::Continue;
        }
    }
  }
  return StateAction::Hold;
}
Manager::StateAction Manager::state_checksum_() {
  if ((uint8_t) (this->checksum_ + *this->buf_read_++)) {
    ESP_LOGE(this->logtag_, "TEXT FRAME: invalid checksum");
  } else {
    if (!this->connected_)
      this->on_connected_();
    ESP_LOGD(this->logtag_, "TEXT FRAME: processing");
    this->on_textframe_();
  }
  this->state_func_ = &Manager::state_idle_;
  size_t size_to_move = this->buf_write_ - this->buf_read_;
  if (size_to_move) {
    // some data are already in the buffer to be processed.
    // we then move those to the beginning and continue processing.
    // This might be expensive but we try not to loose any data
    // and keep the buffer parsing easy. This should be rare anyway.
    ESP_LOGV(this->logtag_, "TEXT FRAME: realigning buffer (move_size=%d)", size_to_move);
    memmove(this->buf_, this->buf_read_, size_to_move);
    this->buf_read_ = this->buf_;
    this->buf_write_ = this->buf_ + size_to_move;
    return StateAction::Continue;
  } else {
    return StateAction::Flush;
  }
}
Manager::StateAction Manager::state_hex_() {
  /*
  this->buf_read_ points to the ':' starting the HEX message
  */
  uint8_t *hex_end = std::find(this->buf_read_, this->buf_write_, '\n');
  if (hex_end == this->buf_write_)
    return StateAction::Hold;

  *hex_end = 0;
  auto hexframe_value = std::string((const char *) (this->buf_read_ + 1));
  auto hf = HexRxFrame(hexframe_value.c_str());
  ESP_LOGD(this->logtag_, "HEX FRAME: received %s (%s)", hexframe_value.c_str(), hf.valid() ? "valid" : "invalid");
  this->hexframe_callback_.call(hexframe_value, hf.valid());
  if (hf.valid()) {
    if (!this->connected_)
      this->on_connected_();
    if (this->rawhexframe_)
      this->rawhexframe_->publish_state(hexframe_value);
    this->on_hexframe_(hf);
  }
  // now cleanup the buffer since it might be carrying a TEXT frame
  ++hex_end;
  size_t hex_size = hex_end - this->buf_read_;
  size_t size_to_move = this->buf_write_ - hex_end;
  ESP_LOGV(this->logtag_, "HEX FRAME: realigning buffer (hex_size=%d move_size=%d)", hex_size, size_to_move);
  if (size_to_move) {
    memmove(this->buf_read_, hex_end, size_to_move);
  }
  this->buf_write_ -= hex_size;
  this->state_func_ = this->state_func_hex_backup_;
  ESP_LOGV(this->logtag_, "HEX FRAME: exit");
  return (this->buf_read_ < this->buf_write_) ? StateAction::Continue : StateAction::Hold;
}
Manager::StateAction Manager::enter_state_hex_() {
  // HEX frames might be interleaved with TEXT ones
  // we're then trying to either parse immediately,
  // or save the state (kind of stack) for further
  // processing when more data will arrive
  ESP_LOGV(this->logtag_, "HEX FRAME: enter");
  this->state_func_hex_backup_ = this->state_func_;
  this->state_func_ = &Manager::state_hex_;
  return StateAction::Continue;
}

void Manager::on_connected_() {
  ESP_LOGD(this->logtag_, "LINK: connected");
  this->connected_ = true;
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(true);
  }
  if (this->auto_create_hex_entities_ || this->hex_registers_.size()) {
    this->send_hexframe(HexTxFrame(HexFrame::Command::Ping));
  }
}

void Manager::on_disconnected_() {
  ESP_LOGD(this->logtag_, "LINK: disconnected");
  this->connected_ = false;
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(false);
  }

  // we should set all entities state to HA 'unavailable' but
  // ESPHOME right now doesn't support it
}
void Manager::on_hexframe_(const HexRxFrame &hexframe) {
  switch (hexframe.command()) {
    case HexFrame::Command::Get:
    case HexFrame::Command::Set:
    case HexFrame::Command::Async: {
      if (hexframe.data_size() > 0) {
        register_id_t hex_id = hexframe.register_id();
        auto entity_iter = this->hex_registers_.find(hex_id);
        if (entity_iter == this->hex_registers_.end()) {
          if (this->auto_create_hex_entities_) {
            ESP_LOGD(this->logtag_, "Looking-up entity for VE.Direct hex register: %04X", (int) hex_id);
            auto hex_register = Factory::build_register(this, hex_id);
            hex_register->dynamic_register();
            hex_register->parse_hex_value(&hexframe);
          }
        } else {
          entity_iter->second->parse_hex_value(&hexframe);
        }
      } else {
        ESP_LOGE(this->logtag_, "Inconsistent hex frame size: %s", hexframe.src);
      }
    }
  }
}

void Manager::on_textframe_() {
  for (uint8_t i = 0; i < this->record_count_; ++i) {
    auto entity_iter = this->text_entities_.find(this->name_[i]);
    if (entity_iter == this->text_entities_.end()) {
      if (this->auto_create_text_entities_) {
        ESP_LOGD(this->logtag_, "Looking-up entity for VE.Direct text field: %s", this->name_[i]);
        auto entity = Factory::build_entity(this, this->name_[i]);
        entity->dynamic_register();
        entity->parse_text_value(this->value_[i]);
      }
    } else {
      entity_iter->second->parse_text_value(this->value_[i]);
    }
  }

  if (auto rawtextframe = this->rawtextframe_) {
    std::string textframe_value;
    for (uint8_t i = 0; i < this->record_count_; ++i) {
      textframe_value.append(this->name_[i]);
      textframe_value.append(":");
      textframe_value.append(this->value_[i]);
      textframe_value.append(",");
    }
    if (rawtextframe->raw_state != textframe_value) {
      rawtextframe->publish_state(textframe_value);
    }
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
