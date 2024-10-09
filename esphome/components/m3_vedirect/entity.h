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
#define NO_TEXT_NAME nullptr

class Manager;

/// @brief Provides an abstraction for an HEX register id. In case of simple registers
/// @brief it also represents the associated entities while, for registers carrying
/// @brief multiple data parameters, it takes the responsibility of splitting those data
/// @brief and forwarding to the correct entity
class HexRegister {
 public:
  Manager *const manager;

  register_id_t id;
  HexRegister(Manager *manager, register_id_t id);

  virtual void dynamic_register(){};

  virtual void parse_hex_value(const HexFrame *hexframe){};

 protected:
};

class VEDirectEntity {
 public:
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

  /// @brief Called when an entity is dynamically initialized by the Manager loop.
  /// @brief This will in turn call the proper register function against App/api
  virtual void dynamic_register(){};
};

class HFEntity : public HexRegister, public VEDirectEntity {
 public:
 protected:
  HFEntity(Manager *manager, register_id_t id) : HexRegister(manager, id) {}
};

class TFEntity : public VEDirectEntity {
 public:
  struct DEF;
  typedef TFEntity *(*entity_initializer_func_t)(Manager *manager, const char *label, const DEF *def);
  struct DEF {
    const char *description;
    const entity_initializer_func_t init;
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

  typedef std::unordered_map<const char *, const DEF, cstring_hash, cstring_eq> def_map_t;
  static const def_map_t DEFS;
  static const DEF *get_def(const char *label) {
    auto entity_def = DEFS.find(label);
    return entity_def == DEFS.end() ? nullptr : &entity_def->second;
  }
  /// @brief Class factory method to auto generate an entity based off the TEXT frame label
  /// @param manager
  /// @param label
  /// @return
  static TFEntity *build(Manager *manager, const char *label);

  const char *const label;
  const DEF *const def;

  /// @brief store the current text value from VE.Direct TEXT frame
  /// @param value the raw data carried by VE.Direct
  virtual void parse_text_value(const char *text_value){};

 protected:
  TFEntity(Manager *manager, const char *label, const DEF *def);
};

}  // namespace m3_vedirect
}  // namespace esphome
