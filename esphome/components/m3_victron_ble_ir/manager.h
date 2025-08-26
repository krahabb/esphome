#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#include <array>
#include <vector>
#include <functional>

#include "defines.h"
#include "protocol.h"
#include "entity.h"

#ifdef USE_ESP32
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include <aes/esp_aes.h>
#endif

namespace esphome {
namespace m3_victron_ble_ir {

#ifdef USE_BINARY_SENSOR
class VBIBinarySensor;
#endif
#ifdef USE_SENSOR
class VBISensor;
#endif
#ifdef USE_TEXT_SENSOR
class VBITextSensor;
#endif

#define MANAGER_ENTITY(type, name) \
 protected: \
  type *name##_{}; /* NOLINT */ \
\
 public: \
  void set_##name(type *name) { /* NOLINT */ \
    this->name##_ = name; \
  }

#ifdef USE_ESP32
class Manager : public esp32_ble_tracker::ESPBTDeviceListener, public Component {
#else
class Manager : public Component {
#endif
 public:
  void dump_config() override;
  void setup() override;
#ifdef DEBUG_VBIENTITY
  void loop() override;
#endif

  // ensure this component is setup before ApiServer in order
  // to dynamically register entities before ApiServer is setup
  float get_setup_priority() const override { return setup_priority::DATA; }

#ifdef USE_ESP32
  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;
#endif

  void set_address(uint64_t address) {
    this->address_ = address;
    sprintf(this->address_str_, "%02X:%02X:%02X:%02X:%02X:%02X", (uint8_t) (address >> 40) & 0xff,
            (uint8_t) (address >> 32) & 0xff, (uint8_t) (address >> 24) & 0xff, (uint8_t) (address >> 16) & 0xff,
            (uint8_t) (address >> 8) & 0xff, (uint8_t) (address >> 0) & 0xff);
  }
  const char *address_str() const { return this->address_str_; }
  void set_bindkey(std::array<uint8_t, 16> key) { this->bindkey_ = key; }

  void add_on_message_callback(std::function<void(const VBI_RECORD *)> callback) {
    this->on_message_callback_.add(std::move(callback));
  }

  void set_auto_create_entities(VBI_RECORD::HEADER::TYPE value) {
    this->auto_create_entities_ = true;
    this->auto_create_type_ = value;
  }

#ifdef USE_BINARY_SENSOR
  MANAGER_ENTITY(binary_sensor::BinarySensorInitiallyOff, link_connected)
#endif
  void set_link_connected_timeout(uint32_t timeout_seconds) { this->link_connected_timeout_ = timeout_seconds * 1000; }

  void register_entity(VBIEntity *entity) { this->entities_.push_back(entity); }

  VBIEntity *lookup_entity_type(VBIEntity::TYPE type);
#ifdef USE_TEXT_SENSOR
  VBITextSensor *lookup_text_sensor_type(VBIEntity::TYPE type);
#endif

 protected:
  uint64_t address_{};
  char address_str_[18] = {};
  std::array<uint8_t, 16> bindkey_;
#ifdef USE_ESP32
  esp_aes_context aes_ctx_;
#endif

  std::vector<VBIEntity *> entities_;

  bool init_entities_{true};
  void init_(VBI_RECORD::HEADER::TYPE record_type);

  bool auto_create_entities_{false};
  VBI_RECORD::HEADER::TYPE auto_create_type_{VBI_RECORD::HEADER::TYPE::AUTO};
  void auto_create_(VBI_RECORD::HEADER::TYPE record_type);

  VBI_RECORD record_{};
  CallbackManager<void(const VBI_RECORD *)> on_message_callback_{};

  uint32_t link_connected_timeout_{};
#ifdef USE_BINARY_SENSOR
  void timeout_link_connected_();
#endif
#ifdef DEBUG_VBIENTITY
  uint32_t time_loop_{};
#endif
};

class MessageTrigger : public Trigger<const VBI_RECORD *> {
 public:
  explicit MessageTrigger(Manager *manager) {
    manager->add_on_message_callback([this](const VBI_RECORD *message) { this->trigger(message); });
  }
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome
