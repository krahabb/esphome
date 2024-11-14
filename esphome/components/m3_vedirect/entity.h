#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include "defines.h"
#include "ve_hexframe.h"
#include "hexregister.h"

namespace esphome {
namespace m3_vedirect {

class Entity : public HexRegister {
 public:
 protected:
  friend class Manager;

  Entity(parse_hex_func_t parse_hex_func = parse_hex_empty_, parse_text_func_t parse_text_func = parse_text_empty_)
      : HexRegister(parse_hex_func, parse_text_func) {}

  /// @brief Called when an entity is dynamically initialized by the Manager loop.
  /// This will in turn call the proper register function against App/api
  virtual void dynamic_register_(){};
};

/// @brief Mixin style specialization for entities that can write configuration data to
/// the VEDirect interface (Number, Select, Switch)
class ConfigEntity {
 public:
  Manager *const manager;
  ConfigEntity(Manager *manager) : manager(manager) {}
};

/// @brief Mixin style specialization for Number and Sensor entities.
class NumericEntity {
 public:
  static const char *UNIT_TO_DEVICE_CLASS[REG_DEF::UNIT::UNIT_COUNT];

 protected:
  float hex_scale_{1.};
};

}  // namespace m3_vedirect
}  // namespace esphome
