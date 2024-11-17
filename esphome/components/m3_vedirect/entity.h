#pragma once

#include "defines.h"
#include "ve_hexframe.h"
#include "hexregister.h"

#include "esphome/core/entity_base.h"

namespace esphome {
namespace m3_vedirect {

class Entity : public HexRegister {
 public:
  // Setup a dynamic registration system for the purpose of dynamically
  // creating entities in the Manager should it be configured so.
  // In order to build a specific entity type, it's compilation unit must be
  // added to the build and this is done in yaml by explicitly setting
  // the corresponding platform entry. This allows to completely disable
  // some (or all) of the platforms and save some code size by
  // not including the unneeded compilation units. When a platform is added
  // through yaml generation, the generator will also add a registration
  // for the corresponding entity 'build_entity' function so that the Manager
  // will be able to instantiate the correct entity. By default, we setup a
  // build function for a plain base 'Entity' object for every specialized type
  // so that the Manager will always instantiate an object (which will have no
  // behavior other than working as a stub in this case).
  typedef Entity *(*build_entity_func_t)(Manager *manager, const char *name, const char *object_id);
  enum Platform {
    BinarySensor,
    Number,
    Select,
    Sensor,
    Switch,
    TextSensor,
    Platform_COUNT,
  };
  static void register_platform(Platform platform, build_entity_func_t build_entity_func) {
    BUILD_ENTITY_FUNC[platform] = build_entity_func;
  }

  // Called by Manager initializer (should just be called once but no harm if multiple invocations)
  // This will 'fix' missing platforms registration by filling in with the most appropriate
  // build_entity_func_t for the case.
  static void update_platforms();

 protected:
  friend class Manager;

  Entity(parse_hex_func_t parse_hex_func = parse_hex_empty_, parse_text_func_t parse_text_func = parse_text_empty_)
      : HexRegister(parse_hex_func, parse_text_func) {}

  static build_entity_func_t BUILD_ENTITY_FUNC[Platform_COUNT];
  static Entity *build_entity(Manager *manager, const char *name, const char *object_id) { return new Entity(); }
  // Called by the actual platform implementation to setup its EntityBase properties
  static void dynamic_init_entity_(EntityBase *entity, const char *name, const char *object_id,
                                   const char *manager_name, const char *manager_id);
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
