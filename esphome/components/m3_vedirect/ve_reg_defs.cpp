#include "ve_reg_defs.h"
#include <algorithm>
#include <cstring>

namespace m3_ve_reg {

const char *ENUM_DEF::lookup_label(enum_t value) {
  auto lookup_def_it = std::lower_bound(this->LOOKUPS.begin(), this->LOOKUPS.end(), value);
  return (lookup_def_it != this->LOOKUPS.end()) && (lookup_def_it->value == value) ? lookup_def_it->label : nullptr;
}

const ENUM_DEF::LOOKUP_DEF *ENUM_DEF::lookup_value(const char *label) {
  for (auto lookup_def_it = this->LOOKUPS.begin(); lookup_def_it != this->LOOKUPS.end(); ++lookup_def_it) {
    if (0 == strcmp(lookup_def_it->label, label))
      return &*lookup_def_it;
  }
  return nullptr;
}

ENUM_DEF::LOOKUP_RESULT ENUM_DEF::get_lookup(enum_t value) {
  LOOKUP_RESULT result;
  auto lookup_def_it = std::lower_bound(this->LOOKUPS.begin(), this->LOOKUPS.end(), value);
  result.index = lookup_def_it - this->LOOKUPS.begin();
  if ((lookup_def_it == this->LOOKUPS.end()) || (lookup_def_it->value != value)) {
    char *label = new char[5];
    sprintf(label, "0x%02X", (int) value);
    this->LOOKUPS.insert(lookup_def_it, {value, label});
    result.lookup_def = &this->LOOKUPS[result.index];
    result.added = true;
  } else {
    result.lookup_def = &*lookup_def_it;
    result.added = false;
  }
  return result;
}

const char *REG_DEF::UNITS[] = {
    "", "A", "V", "VA", "W", "Ah", "kWh", "%", "min", "°C",
};
const float REG_DEF::DIGITS_TO_SCALE[] = {1.f, .1f, .01f, .001f};

// define the enum helpers structs for ENUM registers
#define _ENUMS_LOOKUP_ITEM(enum, value) \
  { value, #enum }
#define DEFINE_ENUMS_BITMASK(register_id, label, access, ...) \
  BITMASK_DEF VE_REG_##label##_BITMASK_DEF = {{BITMASK_##label(_ENUMS_LOOKUP_ITEM)}};
#define DEFINE_ENUMS_BITMASK_S(...)
#define DEFINE_ENUMS_ENUM(register_id, label, access) \
  ENUM_DEF VE_REG_##label##_ENUM_DEF = {{ENUM_##label(_ENUMS_LOOKUP_ITEM)}};
#define DEFINE_ENUMS_NUMERIC(...)
REGISTERS_COMMON(DEFINE_ENUMS)

// define the registers definitions (will be stored in REG_DEF::DEFS)
#define DEFINE_DEFS_BITMASK(register_id, label, access, type) \
  {register_id, #label, REG_DEF::access, HEXFRAME::DATA_TYPE_OF<type>(), &VE_REG_##label##_BITMASK_DEF},
#define DEFINE_DEFS_BITMASK_S(register_id, label, access, type, bitmask_label) \
  {register_id, #label, REG_DEF::access, HEXFRAME::DATA_TYPE_OF<type>(), &VE_REG_##bitmask_label##_BITMASK_DEF},
#define DEFINE_DEFS_ENUM(register_id, label, access) {register_id, #label, REG_DEF::access, &VE_REG_##label##_ENUM_DEF},
#define DEFINE_DEFS_NUMERIC(register_id, label, access, type, digits, unit) \
  {register_id, \
   #label, \
   REG_DEF::access, \
   HEXFRAME::DATA_TYPE_OF<type>(), \
   REG_DEF::digits, \
   REG_DEF::unit, \
   REG_DEF::numeric_to_float_t<type, REG_DEF::digits>},
const REG_DEF REG_DEF::DEFS[REG_DEF::TYPE::_COUNT] = {REGISTERS_COMMON(DEFINE_DEFS)};

const REG_DEF *REG_DEF::find_register_id(register_id_t register_id) {
  const REG_DEF *reg_def_end = DEFS + ARRAY_COUNT(DEFS);
  auto reg_def_it = std::lower_bound(DEFS, reg_def_end, register_id);
  return (reg_def_it != reg_def_end) && (reg_def_it->register_id == register_id) ? reg_def_it : nullptr;
}

#define DEFINE_TEXT_DEF_BITMASK(label, register_type, name) {label, name, REG_DEF::TYPE::register_type},
#define DEFINE_TEXT_DEF_BOOLEAN(label, register_type, name) \
  {label, name, REG_DEF::TYPE::register_type, REG_DEF::CLASS::BOOLEAN},
#define DEFINE_TEXT_DEF_ENUM(label, register_type, name) {label, name, REG_DEF::TYPE::register_type},
#define DEFINE_TEXT_DEF_NUMERIC(label, register_type, name, unit, digits) \
  {label, name, REG_DEF::TYPE::register_type, REG_DEF::UNIT::unit, REG_DEF::DIGITS::digits},
#define DEFINE_TEXT_DEF_STRING(label, register_type, name) \
  {label, name, REG_DEF::TYPE::register_type, REG_DEF::CLASS::STRING},

const TEXT_DEF TEXT_DEF::DEFS[] = {TEXTRECORDS(DEFINE_TEXT_DEF)};
/*
    DEF_TFSENSOR("AC_OUT_I", 0xFFFF, "AC output current", A, D_1),
    DEF_TFSENSOR("AC_OUT_S", 0xFFFF, "AC output apparent power", VA, D_0),
    DEF_TFSENSOR("AC_OUT_V", 0xFFFF, "AC output voltage", V, D_2),
    DEF_TFTEXTSENSOR("AR", 0xFFFF, "Alarm reason"),
    DEF_TFBINARYSENSOR("Alarm", 0xFFFF, "Alarm"),
    DEF_TFTEXTSENSOR("CS", 0xFFFF, "State of operation"),
    DEF_TFTEXTSENSOR("ERR", 0xFFFF, "Error code"),
    DEF_TFTEXTSENSOR("FW", 0xFFFF, "Firmware version (FW)"),
    DEF_TFTEXTSENSOR("FWE", 0xFFFF, "Firmware version (FWE)"),
    DEF_TFSENSOR("H19", 0xFFFF, "Yield total", kWh, D_2),
    DEF_TFSENSOR("H20", 0xFFFF, "Yield today", kWh, D_2),
    DEF_TFSENSOR("H21", 0xFFFF, "Maximum power today", W, D_0),
    DEF_TFSENSOR("H22", 0xFFFF, "Yield yesterday", kWh, D_2),
    DEF_TFSENSOR("H23", 0xFFFF, "Maximum power yesterday", W, D_0),
    DEF_TFSENSOR("I", 0xFFFF, "Battery current", A, D_3),
    DEF_TFSENSOR("IL", 0xFFFF, "Load current", A, D_3),
    DEF_TFBINARYSENSOR("LOAD", 0xFFFF, "Output state"),
    DEF_TFTEXTSENSOR("MODE", 0xFFFF, "Device mode"),
    DEF_TFTEXTSENSOR("MPPT", 0xFFFF, "Tracker operation mode"),
    DEF_TFTEXTSENSOR("OR", 0xFFFF, "Off reason"),
    DEF_TFTEXTSENSOR("PID", 0xFFFF, "Product Id"),
    DEF_TFSENSOR("PPV", 0xFFFF, "PV power", W, D_0),
    DEF_TFTEXTSENSOR("Relay", 0xFFFF, "Relay state"),
    DEF_TFTEXTSENSOR("SER#", 0xFFFF, "Serial number"),
    DEF_TFSENSOR("V", 0xFFFF, "Battery voltage", V, D_3),
    DEF_TFSENSOR("VPV", 0xFFFF, "PV voltage", V, D_3),
    DEF_TFTEXTSENSOR("WARN", 0xFFFF, "Warning reason"),
};
*/

const TEXT_DEF *TEXT_DEF::find_label(const char *label) {
  const TEXT_DEF *it_end = DEFS + ARRAY_COUNT(DEFS);
  auto it = std::lower_bound(DEFS, it_end, label);
  return (it != it_end) && (strcmp(it->label, label) == 0) ? it : nullptr;
}

}  // namespace m3_ve_reg