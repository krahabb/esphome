#include "ve_reg_defs.h"
#include <algorithm>
#include <string.h>

namespace m3_ve_reg {

const char *ENUM_DEF::lookup_label(enum_type value) {
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

ENUM_DEF::LOOKUP_RESULT ENUM_DEF::get_lookup(enum_type value) {
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
#define DEFINE_ENUMS_ENUM(register_id, label, access) \
  ENUM_DEF VE_REG_##label##_ENUM_DEF = {{ENUM_##label(_ENUMS_LOOKUP_ITEM)}};
#define DEFINE_ENUMS_NUMERIC(...)
REGISTERS_COMMON(DEFINE_ENUMS)

// define the registers definitions (will be stored in REG_DEF::DEFS)
#define DEFINE_DEFS_BITMASK(register_id, label, access, type) \
  {register_id, #label, REG_DEF::access, HEXFRAME::DATA_TYPE_OF<type>(), &VE_REG_##label##_BITMASK_DEF},
#define DEFINE_DEFS_ENUM(register_id, label, access) {register_id, #label, REG_DEF::access, &VE_REG_##label##_ENUM_DEF},
#define DEFINE_DEFS_NUMERIC(register_id, label, access, type, digits, unit) \
  {register_id, \
   #label, \
   REG_DEF::access, \
   HEXFRAME::DATA_TYPE_OF<type>(), \
   REG_DEF::digits, \
   REG_DEF::unit, \
   REG_DEF::numeric_to_float_t<type, REG_DEF::digits>},
const REG_DEF REG_DEF::DEFS[] = {REGISTERS_COMMON(DEFINE_DEFS)};

const REG_DEF *REG_DEF::find(register_id_t register_id) {
  const REG_DEF *reg_def_end = DEFS + ARRAY_COUNT(DEFS);
  auto reg_def_it = std::lower_bound(DEFS, reg_def_end, register_id);
  return (reg_def_it != reg_def_end) && (reg_def_it->register_id == register_id) ? reg_def_it : nullptr;
}
}  // namespace m3_ve_reg