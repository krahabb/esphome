#include "ve_reg_defs.h"
#include <algorithm>

namespace ve_reg {

const char *ENUM_DEF::lookup(data_type value, const LOOKUP_DEF *lookup, const LOOKUP_DEF *lookup_end) {
  lookup = std::lower_bound(lookup, lookup_end, value);
  return (lookup != lookup_end) && (lookup->value == value) ? lookup->label : nullptr;
}

const char *REG_DEF::UNITS[] = {
    "", "A", "V", "VA", "W", "Ah", "kWh", "%", "min", "°C",
};
const float REG_DEF::DIGITS_TO_SCALE[] = {1.f, .1f, .01f, .001f};

// define the enum helpers structs for ENUM registers
#define _ENUMS_LOOKUP_ITEM(enum, value) \
  { value, #enum }
#define DEFINE_ENUMS_ENUM(register_id, label, access) \
  const ENUM_DEF::LOOKUP_DEF VE_REG_##label##_ENUM::LOOKUP[] = {ENUM_##label(_ENUMS_LOOKUP_ITEM)}; \
  const ENUM_DEF::LOOKUP_DEF *const VE_REG_##label##_ENUM::LOOKUP_END = \
      VE_REG_##label##_ENUM::LOOKUP + ARRAY_COUNT(VE_REG_##label##_ENUM::LOOKUP);
#define DEFINE_ENUMS_NUMERIC(...)
REGISTERS_COMMON(DEFINE_ENUMS)

// define the registers definitions (will be stored in REG_DEF::DEFS)
#define DEFINE_DEFS_ENUM(register_id, label, access) \
  {register_id, #label, REG_DEF::access, VE_REG_##label##_ENUM::lookup},
#define DEFINE_DEFS_NUMERIC(register_id, label, access, type, digits, unit) \
  {register_id, \
   #label, \
   REG_DEF::access, \
   REG_DEF::DATA_TYPE_OF<type>(), \
   REG_DEF::digits, \
   REG_DEF::unit, \
   REG_DEF::numeric_to_float_t<type, REG_DEF::digits>},
const REG_DEF REG_DEF::DEFS[] = {REGISTERS_COMMON(DEFINE_DEFS)};

const REG_DEF *REG_DEF::find(register_id_t register_id) {
  const REG_DEF *reg_def_end = DEFS + ARRAY_COUNT(DEFS);
  auto reg_def_it = std::lower_bound(DEFS, reg_def_end, register_id);
  return (reg_def_it != reg_def_end) && (reg_def_it->register_id == register_id) ? reg_def_it : nullptr;
}
}  // namespace ve_reg