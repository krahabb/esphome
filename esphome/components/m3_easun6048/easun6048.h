#pragma once

#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

namespace esphome {
namespace m3_easun6048 {
// ------------ Common constants ------------
byte constexpr RX_BUFFER_SIZE = 64;
byte constexpr STATUS_PAYLOAD_SIZE = 16;
byte constexpr CONFIG_PAYLOAD_SIZE = 10;
// Consider the link broken when not receiving a valid frame over this period
int32_t constexpr RX_TIMEOUT_MS = 2000;
// Maximum delay between the last received data and the start of a transmitted
// frame to avoid collision with the display transmission (my controller
// seems to request data every 600ms)
int32_t constexpr TX_SAFE_DELAY_MS = 400;
extern const std::string STATE_UNAVAILABLE;
extern const std::string STATE_UNKNOWN;
extern const char *const TAG;

// Redefine byteswap/convert_big_endian for our needs (from helpers.h)
// This is needed since it looks like __builtin_bswap16 hangs when the operand is
// not evenly aligned in memory (e.g. a byte array offset by 13 bytes)
// at least for Arduino/ESP8266 (EspHome 2025.9.0-dev with framework-arduinoespressif8266 @ 3.30102.0 (3.1.2))
template<typename T> constexpr T byteswap(const byte *data) {
  T m;
  for (size_t i = 0; i < sizeof(T); i++)
    reinterpret_cast<uint8_t *>(&m)[i] = data[sizeof(T) - 1 - i];
  return m;
}
template<> constexpr uint8_t byteswap(const byte *data) { return *data; }
template<> constexpr uint16_t byteswap(const byte *data) { return uint16_t(data[0]) << 8 | uint16_t(data[1]); }
template<> constexpr uint32_t byteswap(const byte *data) {
  return uint32_t(data[0]) << 24 | uint32_t(data[1]) << 16 | uint32_t(data[2]) << 8 | uint32_t(data[3]);
}
template<> constexpr int8_t byteswap(const byte *data) { return *data; }
template<> constexpr int16_t byteswap(const byte *data) { return byteswap<uint16_t>(data); }
template<> constexpr int32_t byteswap(const byte *data) { return byteswap<uint32_t>(data); }

template<typename T> constexpr T convert_big_endian(const byte *data) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return byteswap<T>(data);
#else
  return *reinterpret_cast<T *>(data);
#endif
}

#pragma pack(push, 1)

enum FrameMarker : byte {
  HEADER = 125,
  FOOTER = 13,
};

enum FrameType : byte {
  SET = 1,
  CONFIG = 2,
  STATUS = 3,
};
byte constexpr FRAME_TYPE_CONTEXT(byte t) { return t - 1; }  // 0=SET, 1=CONFIG, 2=STATUS
byte constexpr FRAME_CONTEXT_COUNT = 3;

template<int PAYLOAD_SIZE> union Frame {
  byte raw[PAYLOAD_SIZE + 4];
  struct {
    byte header;
    byte payload_size;
    union {
      FrameType type;
      byte payload[PAYLOAD_SIZE];
    } __attribute__((packed));
    byte checksum;
    byte footer;
  } __attribute__((packed));

  static byte calc_checksum(const byte *data) {
    u8 checksum = *data++;  // length
    const byte *data_end = data + checksum;
    for (; data < data_end; ++data)
      checksum += *data;
    return checksum;
  }
};
#pragma pack(pop)

typedef Frame<1> GenericFrame;

class Easun6048;

class Entity {
 public:
  void set_offset(int offset) { this->offset_ = offset; }

 protected:
  friend class Easun6048;
  Easun6048 *component_;
  int offset_;

  virtual void dump_config(const char *prefix, const char *type) {
    ESP_LOGCONFIG(TAG, "%s  Offset: %u", prefix, this->offset_);
  };
  virtual void parse(const GenericFrame *frame) = 0;
  virtual void set_unavailable() = 0;
};

class NumericEntity : public Entity {
 public:
  void set_scale(float scale) { this->scale_ = scale; }

 protected:
  friend class Easun6048;
  float scale_;

  virtual void dump_config(const char *prefix, const char *type) override {
    ESP_LOGCONFIG(TAG, "%s  Scale: %f", prefix, this->scale_);
    Entity::dump_config(prefix, type);
  };
};

class Easun6048 : public PollingComponent, public uart::UARTDevice {
 public:
  Easun6048() : PollingComponent(SCHEDULER_DONT_RUN) {}

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(link_connected)
#endif
#ifdef USE_TEXT_SENSOR
  void set_set_frame_text_sensor(text_sensor::TextSensor *text_sensor) {
    this->rx_frame_context_[FRAME_TYPE_CONTEXT(SET)].text_sensor = text_sensor;
  }
  void set_status_frame_text_sensor(text_sensor::TextSensor *text_sensor) {
    this->rx_frame_context_[FRAME_TYPE_CONTEXT(STATUS)].text_sensor = text_sensor;
  }
  void set_config_frame_text_sensor(text_sensor::TextSensor *text_sensor) {
    this->rx_frame_context_[FRAME_TYPE_CONTEXT(CONFIG)].text_sensor = text_sensor;
  }
#endif

  enum SerialMode {
    MASTER,
    SLAVE,
  };
  void set_serial_mode(SerialMode mode) { this->serial_mode_ = mode; }
  void set_tx_pin(uint8_t pin) { this->tx_pin_ = pin; }
  void set_update_interval(uint32_t update_interval) override {
    switch (this->serial_mode_) {
      case MASTER:
        PollingComponent::set_update_interval(update_interval);
        break;
      default:
        PollingComponent::set_update_interval(SCHEDULER_DONT_RUN);
        break;
    }
  }

  void bind_status_frame_entity(Entity *entity) {
    entity->component_ = this;
    this->rx_frame_context_[FRAME_TYPE_CONTEXT(STATUS)].entities.push_back(entity);
  }
  void bind_config_frame_entity(Entity *entity) {
    entity->component_ = this;
    this->rx_frame_context_[FRAME_TYPE_CONTEXT(CONFIG)].entities.push_back(entity);
  }

  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;

  void send_config(byte offset, byte value);

 protected:
  // Config
  SerialMode serial_mode_;
  uint8_t tx_pin_{UINT8_MAX};

  // State
  enum RXFrameState {
    RX_WAIT_HEADER,
    RX_WAIT_DATA,
  } rx_frame_state_{RX_WAIT_HEADER};
  byte rx_buffer_[RX_BUFFER_SIZE];
  byte *const rx_buffer_end_{rx_buffer_ + sizeof(rx_buffer_)};
  byte *rx_buffer_write_{rx_buffer_};
  byte *rx_buffer_read_{rx_buffer_};

  int32_t rx_last_epoch_{};
  int32_t rx_last_frame_epoch_{};

  struct RXFrameContext {
    Frame<RX_BUFFER_SIZE - 4> rx_frame;
    std::vector<Entity *> entities;
#ifdef USE_TEXT_SENSOR
    text_sensor::TextSensor *text_sensor{nullptr};
#endif
  } rx_frame_context_[FRAME_CONTEXT_COUNT];

  struct ConfigRequest {
    byte offset;
    byte value;
  };
  std::vector<ConfigRequest> pending_config_requests_;
  void flush_config_requests_();
};

#ifdef USE_NUMBER
template<typename T> class Number : public number::Number, public NumericEntity {
 protected:
  void dump_config(const char *prefix, const char *type) override {
    number::log_number(TAG, prefix, type, this);
    NumericEntity::dump_config(prefix, type);
  }

  void parse(const GenericFrame *frame) override {
    if (this->offset_ >= frame->payload_size) {
      ESP_LOGW(TAG, "Offset %d out of range for number %s", this->offset_, this->name_.c_str());
      return;
    }
    float value = convert_big_endian<T>(frame->payload + this->offset_) * this->scale_;
    if (value != this->state)
      this->publish_state(value);
  }

  void set_unavailable() override { this->publish_state(NAN); }

  void control(float value) override {
    this->component_->send_config(this->offset_, static_cast<T>(std::round(value / this->scale_)));
  }
};
#endif  // USE_NUMBER

#ifdef USE_SELECT
class Select : public select::Select, public Entity {
 public:
  void set_options_map(const std::vector<byte> &options_map) { this->options_map_ = std::move(options_map); }

 protected:
  std::vector<byte> options_map_;
  int current_value_{-1};

  void dump_config(const char *prefix, const char *type) override {
    ESP_LOGCONFIG(TAG, "%s%s '%s'", prefix, type, this->name_.c_str());
    Entity::dump_config(prefix, type);
  }

  void parse(const GenericFrame *frame) override {
    if (this->offset_ >= frame->payload_size) {
      ESP_LOGW(TAG, "Offset %d out of range for select %s", this->offset_, this->name_.c_str());
      return;
    }

    byte new_value = frame->payload[this->offset_];
    if (new_value == this->current_value_)
      return;

    this->set_has_state(true);
    this->current_value_ = new_value;
    auto pos = std::find(this->options_map_.begin(), this->options_map_.end(), new_value);
    if (pos == this->options_map_.end()) {
      ESP_LOGW(TAG, "Unknown option value %d for select %s", new_value, this->name_.c_str());
      this->publish_state(STATE_UNKNOWN, -1);
    } else {
      int index = std::distance(this->options_map_.begin(), pos);
      this->publish_state(this->traits.get_options()[index], index);
    }
  }

  void set_unavailable() override {
    this->set_has_state(false);
    this->current_value_ = -1;
    this->publish_state(STATE_UNAVAILABLE, -1);
  }

  void control(const std::string &value) override {
    auto &options = this->traits.get_options();
    auto pos = std::find(options.begin(), options.end(), value);
    if (pos == options.end()) {
      return;
    }
    int index = std::distance(options.begin(), pos);
    byte new_value = this->options_map_[index];
    if (new_value == this->current_value_) {
      return;
    }
    this->current_value_ = -1;  // force next 'parse' to resend state
    this->component_->send_config(this->offset_, new_value);
  }

  // Override to provide index as well and 'fool' the base class
  void publish_state(const std::string &state, int index) {
    this->state = state;
    ESP_LOGD(TAG, "'%s': Sending state %s (index %d)", this->name_.c_str(), state.c_str(), index);
    this->state_callback_.call(state, index);
  }
};
#endif  // USE_SELECT

#ifdef USE_SENSOR
template<typename T> class Sensor : public sensor::Sensor, public NumericEntity {
 protected:
  void dump_config(const char *prefix, const char *type) override {
    sensor::log_sensor(TAG, prefix, type, this);
    NumericEntity::dump_config(prefix, type);
  }

  void parse(const GenericFrame *frame) override {
    if (this->offset_ >= frame->payload_size) {
      ESP_LOGW(TAG, "Offset %d out of range for sensor %s", this->offset_, this->name_.c_str());
      return;
    }
    float value = convert_big_endian<T>(frame->payload + this->offset_) * this->scale_;
    if (value != this->raw_state)
      this->publish_state(value);
  }

  void set_unavailable() override { this->publish_state(NAN); }
};
#endif  // USE_SENSOR

}  // namespace m3_easun6048
}  // namespace esphome
