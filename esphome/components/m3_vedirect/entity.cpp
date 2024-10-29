#include "entity.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "manager.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "m3_vedirect.entity";

const char *Entity::UNIT_TO_DEVICE_CLASS[] = {
    "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};
const sensor::StateClass Entity::UNIT_TO_STATE_CLASS[] = {
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_TOTAL,       sensor::StateClass::STATE_CLASS_TOTAL_INCREASING,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT,
};

#define DEF_TFBINARYSENSOR(name, disabled) \
  { name, Entity::CLASS::BOOLEAN, disabled, UNIT::NONE, DIGITS::D_0 }
#define DEF_TFSENSOR(name, disabled, unit, digits) \
  { name, Entity::CLASS::NUMERIC, disabled, unit, digits }
#define DEF_TFTEXTSENSOR(name, disabled) \
  { name, Entity::CLASS::ENUM, disabled, UNIT::NONE, DIGITS::D_0 }

const Entity::text_def_map_t Entity::TEXT_DEFS{
    {"AC_OUT_I", DEF_TFSENSOR("AC output current", false, UNIT::A, DIGITS::D_1)},
    {"AC_OUT_S", DEF_TFSENSOR("AC output apparent power", false, UNIT::VA, DIGITS::D_0)},
    {"AC_OUT_V", DEF_TFSENSOR("AC output voltage", false, UNIT::V, DIGITS::D_2)},
    {"H19", DEF_TFSENSOR("Yield total", false, UNIT::kWh, DIGITS::D_2)},
    {"H20", DEF_TFSENSOR("Yield today", false, UNIT::kWh, DIGITS::D_2)},
    {"H21", DEF_TFSENSOR("Maximum power today", false, UNIT::W, DIGITS::D_0)},
    {"H22", DEF_TFSENSOR("Yield yesterday", true, UNIT::kWh, DIGITS::D_2)},
    {"H23", DEF_TFSENSOR("Maximum power yesterday", true, UNIT::W, DIGITS::D_0)},
    {"I", DEF_TFSENSOR("Battery current", false, UNIT::A, DIGITS::D_3)},
    {"IL", DEF_TFSENSOR("Load current", false, UNIT::A, DIGITS::D_3)},
    {"PPV", DEF_TFSENSOR("PV power", false, UNIT::W, DIGITS::D_0)},
    {"V", DEF_TFSENSOR("Battery voltage", false, UNIT::V, DIGITS::D_3)},
    {"VPV", DEF_TFSENSOR("PV voltage", false, UNIT::V, DIGITS::D_3)},

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

void Entity::set_text_label(Manager *manager, const char *label) {
  auto text_def_it = TEXT_DEFS.find(label);
  if (text_def_it != TEXT_DEFS.end())
    this->init_text_def_(&text_def_it->second);
  manager->text_entities_.emplace(label, this);
}

void Entity::set_register_id(Manager *manager, register_id_t register_id) {
  auto reg_def = REG_DEF::find(register_id);
  this->set_reg_def(reg_def ? reg_def : new REG_DEF(register_id));
  manager->hex_registers_.emplace(register_id, this);
}

}  // namespace m3_vedirect
}  // namespace esphome
