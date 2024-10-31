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
