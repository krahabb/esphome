#include "factory.h"
#include "./binary_sensor/binary_sensor.h"
#include "./sensor/sensor.h"
#include "./text_sensor/text_sensor.h"

namespace esphome {
namespace m3_vedirect {

/*
static const VEDirectEntityDef ENTITIES_DEF[] = {
    {"V", 0, "Battery voltage", entity_builder<Sensor>, false, "V", 3, 1000},
    {"VPV", 0, "PV voltage", entity_builder<Sensor>, false, "V", 3, 1000},
    {"PPV", 0, "PV power", entity_builder<Sensor>, false, "W", 0, 1},
    {"I", 0, "Battery current", entity_builder<Sensor>, false, "A", 3, 1000},
    {"IL", 0, "Load current", entity_builder<Sensor>, false, "A", 3, 1000},
    {"LOAD", 0, "Output state", entity_builder<BinarySensor>, false},
    {"Alarm", 0, "Alarm", entity_builder<BinarySensor>, false},
    {"Relay", 0, "Relay state", entity_builder<TextSensor>, false},
    {"AR", 0, "Alarm reason", entity_builder<TextSensor>, false},
    {"OR", 0, "Off reason", entity_builder<TextSensor>, false},
    {"H19", 0, "Yield total", entity_builder<Sensor>, true, "kWh", 2, 100},
    {"H20", 0, "Yield today", entity_builder<Sensor>, false, "kWh", 2, 100},
    {"H21", 0, "Maximum power today", entity_builder<Sensor>, false, "W", 0, 1},
    {"H22", 0, "Yield yesterday", entity_builder<Sensor>, true, "kWh", 2, 100},
    {"H23", 0, "Maximum power yesterday", entity_builder<Sensor>, true, "W", 0, 1},
    {"ERR", 0, "Error code", entity_builder<TextSensor>, false},
    {"CS", 0, "State of operation", entity_builder<TextSensor>, false},
    {"FW", 0, "Firmware version (FW)", entity_builder<TextSensor>, true},
    {"FWE", 0, "Firmware version (FWE)", entity_builder<TextSensor>, true},
    {"PID", 0, "Product Id", entity_builder<TextSensor>, true},
    {"SER#", 0, "Serial number", entity_builder<TextSensor>, true},
    {"MODE", 0, "Device mode", entity_builder<TextSensor>, false},
    {"AC_OUT_V", 0, "AC output voltage", entity_builder<Sensor>, false, "V", 2, 100},
    {"AC_OUT_I", 0, "AC output current", entity_builder<Sensor>, false, "A", 1, 10},
    {"AC_OUT_S", 0, "AC output apparent power", entity_builder<Sensor>, false, "VA", 0, 1},
    {"WARN", 0, "Warning reason", entity_builder<TextSensor>, false},
    {"MPPT", 0, "Tracker operation mode", entity_builder<TextSensor>, false},
    {nullptr, 0xEDA8, "Load output state", entity_builder<BinarySensor>, false},
    {nullptr, 0xEDA9, "Load voltage", entity_builder<HexSensor<uint16_t>>, false, "V", 2, 100},
    };
*/

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

/*static*/ TFEntity *Factory::build_entity(Manager *manager, const char *label) {
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
  entity->set_name(label);
  entity->set_disabled_by_default(true);
  return entity;
}

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
  char *object_id = new char[7];
  sprintf(object_id, "0x%04X", (int) id);
  entity->set_object_id(object_id);
  entity->set_name(object_id);
  entity->set_disabled_by_default(true);
  return entity;
}

}  // namespace m3_vedirect
}  // namespace esphome
