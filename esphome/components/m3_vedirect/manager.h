#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include <unordered_map>
#include <string_view>
#include <vector>

#include "defines.h"
#include "hexframe.h"

namespace esphome {
namespace m3_vedirect {

class HexRegister;
class TFEntity;

#define MANAGER_ENTITY_(type, name) \
 protected: \
  type *name##_{}; /* NOLINT */ \
\
 public: \
  void set_##name(type *name) { /* NOLINT */ \
    this->name##_ = name; \
  }

class Manager : public uart::UARTDevice, public Component, protected FrameHandler {
  // dedicated entities to manage component state/behavior
  MANAGER_ENTITY_(text_sensor::TextSensor, rawhexframe)
  MANAGER_ENTITY_(text_sensor::TextSensor, rawtextframe)
  MANAGER_ENTITY_(binary_sensor::BinarySensor, link_connected)
  MANAGER_ENTITY_(sensor::Sensor, run_time)

  const std::string &get_vedirect_id() { return this->vedirect_id_; }
  void set_vedirect_id(const char *vedirect_id) { this->vedirect_id_ = vedirect_id; }
  const std::string &get_vedirect_name() { return this->vedirect_name_; }
  void set_vedirect_name(const char *vedirect_name) { this->vedirect_name_ = vedirect_name; }
  void set_auto_create_text_entities(bool value) { this->auto_create_text_entities_ = value; }
  void set_auto_create_hex_entities(bool value) { this->auto_create_hex_entities_ = value; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  void setup_entity_name_id(EntityBase *entity, const char *name, const char *object_id);

  void send_hexframe(const HexFrame &hexframe);
  void send_hexframe(const char *hexdigits, bool addchecksum);
  static void send_hexframe(const std::string &vedirect_id, const std::string &payload);

 protected:
  // component config
  const char *logtag_;
  std::string vedirect_id_;
  std::string vedirect_name_;
  bool auto_create_text_entities_{true};
  bool auto_create_hex_entities_{true};

  uint32_t ping_retry_timeout_{60000};

  // component state
  bool connected_{false};
  uint32_t millis_last_rx_{0};
  uint32_t millis_last_textframe_rx_{0};
  uint32_t millis_last_hexframe_rx_{0};
  uint32_t millis_last_hexframe_tx_{0};
  uint32_t millis_last_ping_tx_{0};

  inline void on_connected_();
  inline void on_disconnected_();

  // override FrameHandler
  void on_frame_hex_(const HexFrame &hexframe) override;
  void on_frame_text_(TextRecord **text_records, uint8_t text_records_count) override;
  void on_frame_error_(const char *message) override;

  // These will provide 'map' access either by text record name (text_entities_)
  // or by HEX register id (hex_entities_). Since some HEX registers are also
  // published in TEXT frames we're also trying to map these to the same entity.
  friend class TFEntity;
  std::unordered_map<const char *, TFEntity *, cstring_hash, cstring_eq> text_entities_;
  friend class HexRegister;
  std::unordered_map<uint16_t, HexRegister *> hex_registers_;

  friend class HexFrameTrigger;
  CallbackManager<void(const HexFrame &)> hexframe_callback_;
  void add_on_frame_callback(std::function<void(const HexFrame &)> callback) {
    this->hexframe_callback_.add(std::move(callback));
  }

  static std::vector<Manager *> managers_;
};

class HexFrameTrigger : public Trigger<const HexFrame &> {
 public:
  explicit HexFrameTrigger(Manager *vedirect) {
    vedirect->add_on_frame_callback([this](const HexFrame &hexframe) { this->trigger(hexframe); });
  }
};

template<typename... Ts> class HexFrameSendRawAction : public Action<Ts...> {
 public:
  HexFrameSendRawAction() {}
  TEMPLATABLE_VALUE(std::string, vedirect_id)
  TEMPLATABLE_VALUE(std::string, payload)

  void play(Ts... x) { Manager::send_hexframe(this->vedirect_id_.value(x...), this->payload_.value(x...)); }

 protected:
};

}  // namespace m3_vedirect
}  // namespace esphome
