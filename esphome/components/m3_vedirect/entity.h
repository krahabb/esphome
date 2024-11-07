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
  /// @brief Binds the entity to a TEXT FRAME field label so that text frame parsing
  /// will be automatically routed. This method is part of the public interface
  /// called by yaml generaed code
  /// @param label the name of the TEXT FRAME record to bind
  void set_text_label(Manager *manager, const char *label);

 protected:
  friend class Manager;

  Entity(parse_hex_func_t parse_hex_func = parse_hex_empty_, parse_text_func_t parse_text_func = parse_text_empty_)
      : HexRegister(parse_hex_func, parse_text_func) {}

  /// @brief Called when an entity is dynamically initialized by the Manager loop.
  /// This will in turn call the proper register function against App/api
  virtual void dynamic_register_(){};
};

/// @brief Specialization for entities that can write configuration data to
/// the VEDirect interface
class ConfigEntity : public Entity {
 public:
  Manager *const manager;
  ConfigEntity(Manager *manager, parse_hex_func_t parse_hex_func = parse_hex_empty_,
               parse_text_func_t parse_text_func = parse_text_empty_)
      : Entity(parse_hex_func, parse_text_func), manager(manager) {}
};

}  // namespace m3_vedirect
}  // namespace esphome
