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
  /// @brief Together with SUBCLASS defines the data semantics of this entity
  enum CLASS : u_int8_t {
    BOOLEAN,
    ENUMERATION,  // enumeration data or bitmask
    MEASUREMENT,  // numeric data (either signed or unsigned)
  };

  /// @brief Together with CLASS defines the data semantics of this entity
  enum SUBCLASS : u_int8_t {
    BITMASK,   // represents a set of bit flags
    ENUM,      // represents a value among an enumeration
    SELECTOR,  // same as ENUM but used to select conditional parsing
    MEASURE,
    TEMPERATURE,
    CELLVOLTAGE,
  };

  // configuration symbols for numeric sensors
  enum UNIT : u_int8_t {
    A = 0,
    V = 1,
    VA = 2,
    W = 3,
    Ah = 4,
    kWh = 5,
    SOC_PERCENTAGE = 6,
    minute = 7,
    CELSIUS = 8,
    _COUNT,
  };
  static const char *UNITS[UNIT::_COUNT];
  static const char *DEVICE_CLASSES[UNIT::_COUNT];
  static const sensor::StateClass STATE_CLASSES[UNIT::_COUNT];

  enum DIGITS : u_int8_t {
    D_0 = 0,
    D_1 = 1,
    D_2 = 2,
    D_3 = 3,
  };
  static const float DIGITS_TO_SCALE[4];

  struct TEXT_DEF {
    const char *description;
    const CLASS cls : 2;
    const SUBCLASS subcls : 6;
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
  virtual void parse_hex_value(const HexFrame *hexframe){};

  /// @brief Called when an entity is dynamically initialized by the Manager loop.
  /// @brief This will in turn call the proper register function against App/api
  virtual void dynamic_register(){};

 protected:
  VEDirectEntity(Manager *manager) : manager(manager) {}

  /// @brief Preset entity properties based off our TEXT_DEF. This is being called
  /// automatically by VEDirectEntity methods when a proper definition is available.
  /// @param def
  virtual void init_text_def_(const TEXT_DEF *text_def) {}

 private:
  register_id_t register_id_{};

  template<typename TEntity>
  static TEntity *dynamic_build_entity_(Manager *manager, const char *name, const char *object_id);
};

}  // namespace m3_vedirect
}  // namespace esphome
