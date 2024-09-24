#include "protocol_enums.h"

namespace esphome {
namespace m3_victron_ble_ir {

ENUM_LOOKUP_DEF::ENUM_LOOKUP_DEF(u_int32_t value, const char *enum_name) : value(value) {
  // format the enum declared identifier as a pretty string
  // the enum is in the form TOKENA_TOKENB...
  bool start_of_token = true;
  for (const char *c = enum_name; *c != 0; ++c) {
    if (*c == '_') {
      this->label.push_back(' ');
      start_of_token = true;
      continue;
    }
    if (start_of_token) {
      this->label.push_back(*c);
      start_of_token = false;
    } else {
      this->label.push_back(tolower(*c));
    }
  }
}

std::string EnumBase::lookup(u_int32_t value, const ENUM_LOOKUP_DEF *lookup, const ENUM_LOOKUP_DEF *lookup_end) {
  lookup = std::lower_bound(lookup, lookup_end, value);
  if ((lookup != lookup_end) && (lookup->value == value)) {
    return lookup->label;
  }
  return std::to_string(value);
}

#define DEFINE_ENUM(ENUM_MACRO) \
  const ENUM_LOOKUP_DEF ENUM_MACRO::LOOKUP[] = {ENUM_MACRO(DEFINE_ENUM_LOOKUP_ITEM)}; \
  const ENUM_LOOKUP_DEF *const ENUM_MACRO::LOOKUP_END = ENUM_MACRO::LOOKUP + ARRAY_COUNT(ENUM_MACRO::LOOKUP);

DEFINE_ENUM(ENUM_VE_REG_AC_IN_ACTIVE)
DEFINE_ENUM(ENUM_VE_REG_ALARM_NOTIFICATION)
DEFINE_ENUM(ENUM_VE_REG_ALARM_REASON)
DEFINE_ENUM(ENUM_VE_REG_BMS_ERROR)
DEFINE_ENUM(BITMASK_VE_REG_BMS_FLAGS)
DEFINE_ENUM(ENUM_VE_REG_BMV_AUX_INPUT)
DEFINE_ENUM(ENUM_VE_REG_BMV_MONITOR_MODE)
DEFINE_ENUM(ENUM_VE_REG_BALANCER_STATUS)
DEFINE_ENUM(ENUM_VE_REG_CHR_ERROR_CODE)
DEFINE_ENUM(ENUM_VE_REG_DC_OUTPUT_STATUS)
DEFINE_ENUM(ENUM_VE_REG_DEVICE_OFF_REASON_2)
DEFINE_ENUM(ENUM_VE_REG_DEVICE_STATE)
DEFINE_ENUM(ENUM_VE_REG_SMART_LITHIUM_ERROR_FLAGS)
DEFINE_ENUM(ENUM_VE_REG_VEBUS_VEBUS_ERROR)

}  // namespace m3_victron_ble_ir
}  // namespace esphome