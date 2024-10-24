#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <unordered_map>

#include "defines.h"
#include "hexframe.h"

namespace esphome {
namespace m3_vedirect {

#define NO_REGISTER_ID 0

class Manager;

class VEDirectEntity {
 public:
  typedef REG_DEF::CLASS CLASS;

  // configuration symbols for numeric sensors
  typedef REG_DEF::UNIT UNIT;
  static const char *UNIT_TO_DEVICE_CLASS[];
  static const sensor::StateClass UNIT_TO_STATE_CLASS[];

  typedef REG_DEF::DIGITS DIGITS;

  struct TEXT_DEF {
    const char *description;
    const CLASS cls : 2;
    const bool initially_disabled;
    // Optional entity 'class' definitions
    union {
      // Sensor entity definitions
      struct {
        const UNIT unit : 4;
        const DIGITS digits : 2;
      } __attribute__((packed));
    };
  } __attribute__((packed));

  typedef std::unordered_map<const char *, const TEXT_DEF, cstring_hash, cstring_eq> text_def_map_t;
  static const text_def_map_t TEXT_DEFS;
  static const TEXT_DEF *get_text_def(const char *label) {
    auto entity_def = TEXT_DEFS.find(label);
    return entity_def == TEXT_DEFS.end() ? nullptr : &entity_def->second;
  }
  /// @brief Class factory method to auto generate an entity based off the TEXT frame label
  /// @param manager
  /// @param label
  /// @return
  static VEDirectEntity *build(Manager *manager, const char *label);
  static VEDirectEntity *build(Manager *manager, register_id_t register_id);

  Manager *const manager;

  /// @brief Binds the entity to a TEXT FRAME field label so that text frame parsing
  /// will be automatically routed. This method is part of the public interface
  /// called by yaml generaed code
  /// @param label the name of the TEXT FRAME record to bind
  void set_text_label(const char *label);
  /// @brief Parse the current value from VE.Direct TEXT frame and update this entity
  /// @param value the raw data carried by VE.Direct
  virtual void parse_text_value(const char *text_value){};

  void set_register_id(register_id_t register_id);
  register_id_t get_register_id() { return this->register_id_; }

  typedef FrameHandler::RxHexFrame RxHexFrame;
  typedef void (*parse_hex_func_t)(VEDirectEntity *entity, const RxHexFrame *hexframe);
  inline parse_hex_func_t parse_hex() { return this->parse_hex_func_; }

  /// @brief Called when an entity is dynamically initialized by the Manager loop.
  /// @brief This will in turn call the proper register function against App/api
  virtual void dynamic_register(){};

 protected:
  VEDirectEntity(Manager *manager) : manager(manager) {}

  /// @brief Preset entity properties based off our REG_DEF. This is being called
  /// automatically by VEDirectEntity methods when a proper definition is available.
  /// @param reg_def: the proper register definition if available or null to proceed
  /// with some meaningful default initialization
  virtual void init_reg_def_(const REG_DEF *reg_def) {}

  /// @brief Preset entity properties based off our TEXT_DEF. This is being called
  /// automatically by VEDirectEntity methods when a proper definition is available.
  virtual void init_text_def_(const TEXT_DEF *text_def) {}

 protected:
  register_id_t register_id_{};

  HexFrame::DataType hex_data_type_{HexFrame::DataType::unknown};
  uint8_t hex_data_size_{};

  parse_hex_func_t parse_hex_func_{parse_hex_empty_};
  static void parse_hex_empty_(VEDirectEntity *entity, const RxHexFrame *hexframe) {}

  template<typename TEntity>
  static TEntity *dynamic_build_entity_(Manager *manager, const char *name, const char *object_id);
};

}  // namespace m3_vedirect
}  // namespace esphome
