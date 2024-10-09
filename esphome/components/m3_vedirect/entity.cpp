#include "entity.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "manager.h"
#include "binary_sensor/binary_sensor.h"
#include "sensor/sensor.h"
#include "text_sensor/text_sensor.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "m3_vedirect.entity";

HexRegister::HexRegister(Manager *manager, register_id_t id) : manager(manager), id(id) {
  manager->hex_registers_.emplace(id, this);
}

const char *VEDirectEntity::UNITS[] = {
    "A", "V", "VA", "W", "Ah", "kWh", "%", "min", "°C",
};
const char *VEDirectEntity::DEVICE_CLASSES[] = {
    "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};
const sensor::StateClass VEDirectEntity::STATE_CLASSES[UNIT::_COUNT] = {
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_TOTAL,       sensor::StateClass::STATE_CLASS_TOTAL_INCREASING,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT,
};
const float VEDirectEntity::DIGITS_TO_SCALE[] = {1.f, .1f, .01f, .001f};

#define DEF_TFBINARYSENSOR(name, disabled) \
  { name, entity_builder<TFBinarySensor>, disabled }
#define DEF_TFSENSOR(name, disabled, unit, digits) \
  { name, entity_builder<TFSensor>, disabled, unit, digits }
#define DEF_TFTEXTSENSOR(name, disabled) \
  { name, entity_builder<TFTextSensor>, disabled }

template<typename T> TFEntity *entity_builder(Manager *manager, const char *label, const TFEntity::DEF *def) {
  return new T(manager, label, def);
}

const TFEntity::def_map_t TFEntity::DEFS{
    {"AC_OUT_I", DEF_TFSENSOR("AC output current", false, TFEntity::UNIT::A, TFEntity::DIGITS::D_1)},
    {"AC_OUT_S", DEF_TFSENSOR("AC output apparent power", false, TFEntity::UNIT::VA, TFEntity::DIGITS::D_0)},
    {"AC_OUT_V", DEF_TFSENSOR("AC output voltage", false, TFEntity::UNIT::V, TFEntity::DIGITS::D_2)},
    {"H19", DEF_TFSENSOR("Yield total", false, TFEntity::UNIT::kWh, TFEntity::DIGITS::D_2)},
    {"H20", DEF_TFSENSOR("Yield today", false, TFEntity::UNIT::kWh, TFEntity::DIGITS::D_2)},
    {"H21", DEF_TFSENSOR("Maximum power today", false, TFEntity::UNIT::W, TFEntity::DIGITS::D_0)},
    {"H22", DEF_TFSENSOR("Yield yesterday", true, TFEntity::UNIT::kWh, TFEntity::DIGITS::D_2)},
    {"H23", DEF_TFSENSOR("Maximum power yesterday", true, TFEntity::UNIT::W, TFEntity::DIGITS::D_0)},
    {"I", DEF_TFSENSOR("Battery current", false, TFEntity::UNIT::A, TFEntity::DIGITS::D_3)},
    {"IL", DEF_TFSENSOR("Load current", false, TFEntity::UNIT::A, TFEntity::DIGITS::D_3)},
    {"PPV", DEF_TFSENSOR("PV power", false, TFEntity::UNIT::W, TFEntity::DIGITS::D_0)},
    {"V", DEF_TFSENSOR("Battery voltage", false, TFEntity::UNIT::V, TFEntity::DIGITS::D_3)},
    {"VPV", DEF_TFSENSOR("PV voltage", false, TFEntity::UNIT::V, TFEntity::DIGITS::D_3)},

    {"AR", DEF_TFTEXTSENSOR("Alarm reason", false)},
    {"CS", DEF_TFTEXTSENSOR("State of operation", false)},
    {"ERR", DEF_TFTEXTSENSOR("Error code", false)},
    {"FW", DEF_TFTEXTSENSOR("Firmware version (FW)", true)},
    {"FWE", DEF_TFTEXTSENSOR("Firmware version (FWE)", true)},
    {"MODE", DEF_TFTEXTSENSOR("Device mode", false)},
    {"MPPT", DEF_TFTEXTSENSOR("Tracker operation mode", false)},
    {"OR", DEF_TFTEXTSENSOR("Off reason", false)},
    {"PID", DEF_TFTEXTSENSOR("Product Id", true)},
    {"Relay", DEF_TFTEXTSENSOR("Relay state", false)},
    {"SER#", DEF_TFTEXTSENSOR("Serial number", true)},
    {"WARN", DEF_TFTEXTSENSOR("Warning reason", false)},

    {"Alarm", DEF_TFBINARYSENSOR("Alarm", false)},
    {"LOAD", DEF_TFBINARYSENSOR("Output state", false)},

};

TFEntity *TFEntity::build(Manager *manager, const char *label) {
  auto entity_def = DEFS.find(label);
  if (entity_def == DEFS.end()) {
    // ENTITIES_DEF lacks the definition for this parameter so
    // we return a disabled (TextSensor) entity.
    // We allocate a copy since the label param is 'volatile'
    auto entity = new TFTextSensor(manager, strdup(label), nullptr);
    entity->set_disabled_by_default(true);
    return entity;
  } else {
    return entity_def->second.init(manager, entity_def->first, &entity_def->second);
  }
}

TFEntity::TFEntity(Manager *manager, const char *label, const DEF *def) : label(label), def(def) {
  manager->text_entities_.emplace(label, this);
}

}  // namespace m3_vedirect
}  // namespace esphome
