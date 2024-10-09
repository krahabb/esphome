#include "factory.h"
#include "./binary_sensor/binary_sensor.h"
#include "./sensor/sensor.h"
#include "./text_sensor/text_sensor.h"

namespace esphome {
namespace m3_vedirect {

/*REMOVE
template<typename T> TFEntity *entity_builder(Manager *manager, const char *label) { return new T(manager, label); }
typedef TFEntity *(*entity_initializer_func)(Manager *manager, const char *label);
struct TFEntityInit {
  const char *const label;
  const entity_initializer_func init;
  const bool initially_disabled;
};

static const TFEntityInit TFENTITIES_INIT[] = {
    {"AC_OUT_I", entity_builder<TFSensor>, false},    {"AC_OUT_S", entity_builder<TFSensor>, false},
    {"AC_OUT_V", entity_builder<TFSensor>, false},    {"AR", entity_builder<TFTextSensor>, false},
    {"Alarm", entity_builder<TFBinarySensor>, false}, {"CS", entity_builder<TFTextSensor>, false},
    {"ERR", entity_builder<TFTextSensor>, false},     {"FW", entity_builder<TFTextSensor>, true},
    {"FWE", entity_builder<TFTextSensor>, true},      {"H19", entity_builder<TFSensor>, true},
    {"H20", entity_builder<TFSensor>, false},         {"H21", entity_builder<TFSensor>, false},
    {"H22", entity_builder<TFSensor>, true},          {"H23", entity_builder<TFSensor>, true},
    {"I", entity_builder<TFSensor>, false},           {"IL", entity_builder<TFSensor>, false},
    {"LOAD", entity_builder<TFBinarySensor>, false},  {"MODE", entity_builder<TFTextSensor>, false},
    {"MPPT", entity_builder<TFTextSensor>, false},    {"OR", entity_builder<TFTextSensor>, false},
    {"PID", entity_builder<TFTextSensor>, true},      {"PPV", entity_builder<TFSensor>, false},
    {"Relay", entity_builder<TFTextSensor>, false},   {"SER#", entity_builder<TFTextSensor>, true},
    {"V", entity_builder<TFSensor>, false},           {"VPV", entity_builder<TFSensor>, false},
    {"WARN", entity_builder<TFTextSensor>, false},
};

TFEntity *Factory::build_entity(Manager *manager, const char *label) {
  for (const TFEntityInit &_def : TFENTITIES_INIT) {
    int _strcmp = strcmp(label, _def.label);
    if (_strcmp == 0) {
      return _def.init(manager, _def.label);
    } else if (_strcmp < 0) {
      break;
    }
  }
  // ENTITIES_DEF lacks the definition for this parameter so
  // we return a disabled (TextSensor) entity.
  // We allocate a copy for text_name since the param is 'volatile'
  label = strdup(label);
  auto entity = new TFTextSensor(manager, label);
  entity->set_disabled_by_default(true);
  return entity;
}
*/

struct HexRegisterDef;

typedef HexRegister *(*register_initializer_func)(Manager *manager, const HexRegisterDef &);

struct HexRegisterDef {
  /// @brief the VE.Direct register id for entities managed through HEX frames
  const register_id_t id;
  /// @brief Friendly name
  const char *const description;

  const register_initializer_func init;

  HexRegisterDef(register_id_t id, const char *description = nullptr, const register_initializer_func init = nullptr)
      : id(id), description(description), init(init) {}
};

template<typename T> HexRegister *register_builder(Manager *manager, const HexRegisterDef &def) {
  return new T(manager, def);
}

static const HexRegisterDef REGISTERS_DEF[] = {

};

/*static*/ HexRegister *Factory::build_register(Manager *manager, register_id_t id) {
  // check if we have a 'structured' parser or any other special behavior
  for (const HexRegisterDef &_def : REGISTERS_DEF) {
    if (_def.id == id)
      return _def.init(manager, _def);
  }

  // else build a raw text sensor
  auto entity = new HFTextSensor(manager, id);
  entity->set_disabled_by_default(true);
  return entity;
}

}  // namespace m3_vedirect
}  // namespace esphome
