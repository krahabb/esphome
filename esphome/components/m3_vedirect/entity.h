#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <unordered_map>

#include "defines.h"
#include "ve_hexframe.h"
#include "hexregister.h"

namespace esphome {
namespace m3_vedirect {

class Entity : public HexRegister {
 public:
  typedef REG_DEF::CLASS CLASS;

  // configuration symbols for numeric sensors
  typedef REG_DEF::UNIT UNIT;
  static const char *UNIT_TO_DEVICE_CLASS[];
  static const sensor::StateClass UNIT_TO_STATE_CLASS[];

  typedef REG_DEF::DIGITS DIGITS;

  struct TEXT_DEF {
    const char *description;
    const CLASS cls : 3;
    const bool initially_disabled : 1;
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

  /// @brief Binds the entity to a TEXT FRAME field label so that text frame parsing
  /// will be automatically routed. This method is part of the public interface
  /// called by yaml generaed code
  /// @param label the name of the TEXT FRAME record to bind
  void set_text_label(Manager *manager, const char *label);

  void set_register_id(Manager *manager, register_id_t register_id);

 protected:
  friend class Manager;
  /// @brief Called when an entity is dynamically initialized by the Manager loop.
  /// This will in turn call the proper register function against App/api
  virtual void dynamic_register_(){};

  /// @brief Preset entity properties based off our TEXT_DEF. This is being called
  /// automatically by VEDirectEntity methods when a proper definition is available.
  virtual void init_text_def_(const TEXT_DEF *text_def) {}
  /// @brief Called by Manager when a new TEXT FRAME has been received.
  /// Parses the current value from VE.Direct TEXT frame and update this entity.
  /// @param value the raw data carried by VE.Direct
  virtual void parse_text_(const char *text_value){};
};

/// @brief Specialization for entities that can write configuration data to
/// the VEDirect interface
class ConfigEntity : public Entity {
 public:
  Manager *const manager;
  ConfigEntity(Manager *manager) : manager(manager) {}
};

}  // namespace m3_vedirect
}  // namespace esphome
