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
#include "ve_hexframe.h"

namespace esphome {
namespace m3_vedirect {

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
  void set_ping_timeout(uint32_t seconds) { this->ping_timeout_ = seconds * 1000; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  /// @brief Initialize and link the hex_register into the Manager dispatcher system
  /// @param hex_register : the register to be initialized/linked
  /// @param reg_def : the register descriptor definition
  void init_register(HexRegister *hex_register, const REG_DEF *reg_def);
  /// @brief Configure this entity based off our registers grammar (REG_DEF::DEFS).
  /// This method is part of the public interface called by yaml generated code
  /// @param register_type the TYPE enum from our pre-defined registers set
  void init_entity(Entity *entity, REG_DEF::TYPE register_type);
  /// @brief Binds the entity to a TEXT FRAME field label so that text frame parsing
  /// will be automatically routed. This method is part of the public interface
  /// called by yaml generated code
  /// @param label the name of the TEXT FRAME record to bind
  void init_entity(Entity *entity, const char *label);

  static std::vector<Manager *> get_managers(const std::string &vedirect_id);

  void send_hexframe(const HexFrame &hexframe);
  void send_hexframe(const char *rawframe, bool addchecksum = true);
  void send_hexframe(const std::string &rawframe, bool addchecksum = true) {
    this->send_hexframe(rawframe.c_str(), addchecksum);
  }
  void send_command(HEXFRAME::COMMAND command) { this->send_hexframe(HexFrame_Command(command)); }
  void send_register_get(register_id_t register_id) { this->send_hexframe(HexFrame_Get(register_id)); }
  void send_register_set(register_id_t register_id, const void *data, HEXFRAME::DATA_TYPE data_type) {
    this->send_hexframe(HexFrame_Set(register_id, data, data_type));
  }
  template<typename T> void send_register_set(register_id_t register_id, T data) {
    this->send_hexframe(HexFrame_Set(register_id, data));
  }

  class HexFrameTrigger : public Trigger<const HexFrame &> {
   public:
    explicit HexFrameTrigger(Manager *vedirect) {
      vedirect->add_on_frame_callback([this](const HexFrame &hexframe) { this->trigger(hexframe); });
    }
  };

  template<typename... Ts> class BaseAction : public Action<Ts...> {
   public:
    TEMPLATABLE_VALUE(std::string, vedirect_id)
  };

  template<typename... Ts> class Action_send_hexframe : public BaseAction<Ts...> {
   public:
    TEMPLATABLE_VALUE(std::string, data)

    void play(Ts... x) {
      for (auto manager : Manager::get_managers(this->vedirect_id_.value(x...)))
        manager->send_hexframe(this->data_.value(x...));
    }
  };
  template<typename... Ts> class Action_send_command : public BaseAction<Ts...> {
   public:
    TEMPLATABLE_VALUE(uint8_t, command)
    TEMPLATABLE_VALUE(register_id_t, register_id)
    TEMPLATABLE_VALUE(uint32_t, data)
    TEMPLATABLE_VALUE(uint8_t, data_size)

    void play(Ts... x) {
      for (auto manager : Manager::get_managers(this->vedirect_id_.value(x...))) {
        HEXFRAME::COMMAND command = (HEXFRAME::COMMAND) this->command_.value(x...);
        switch (command) {
          case HEXFRAME::COMMAND::Get:
            manager->send_register_get(this->register_id_.value(x...));
            break;
          case HEXFRAME::COMMAND::Set:
            switch (this->data_size_.value(x...)) {
              case 1:
                manager->send_register_set(this->register_id_.value(x...), (uint8_t) this->data_.value(x...));
                break;
              case 2:
                manager->send_register_set(this->register_id_.value(x...), (uint16_t) this->data_.value(x...));
                break;
              default:
                manager->send_register_set(this->register_id_.value(x...), this->data_.value(x...));
                break;
            }
            break;
          default:
            manager->send_command(command);
        }
      }
    }
  };

 protected:
  static std::vector<Manager *> managers_;
  // component config
  const char *logtag_;
  std::string vedirect_id_;
  std::string vedirect_name_;
  bool auto_create_text_entities_{true};
  bool auto_create_hex_entities_{false};

  uint32_t ping_timeout_{0};

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
  void on_frame_hex_(const RxHexFrame &hexframe) override;
  void on_frame_text_(TextRecord **text_records, uint8_t text_records_count) override;
  void on_frame_hex_error_(Error error) override;
  void on_frame_text_error_(Error error) override;

  // These will provide 'map' access either by text record name (text_entities_)
  // or by HEX register id (hex_entities_). Since some HEX registers are also
  // published in TEXT frames we're also trying to map these to the same entity.
  std::unordered_map<const char *, HexRegister *, cstring_hash, cstring_eq> text_entities_;
  typedef std::unordered_map<uint16_t, HexRegister *> hex_registers_t;
  hex_registers_t hex_registers_;

  HexRegister *get_hex_register_(register_id_t register_id, bool create);

  friend class HexFrameTrigger;
  CallbackManager<void(const HexFrame &)> hexframe_callback_;
  void add_on_frame_callback(std::function<void(const HexFrame &)> callback) {
    this->hexframe_callback_.add(std::move(callback));
  }

  /// @brief Class factory method to auto generate an entity based off the TEXT frame label
  /// @param manager
  /// @param label
  /// @return
  HexRegister *build_text_entity_(const char *label);
  HexRegister *build_hex_register_(register_id_t register_id);
  template<typename TEntity> TEntity *dynamic_build_entity_(const char *name, const char *object_id);
  void dynamic_init_entity_(EntityBase *entity, const char *name, const char *object_id);
};

}  // namespace m3_vedirect
}  // namespace esphome
