#pragma once
#include "defines.h"

#include <string>

namespace esphome {
namespace m3_victron_ble_ir {

//
// Helper macros to create 'named' enums in order to provide
// strings representations
//
struct ENUM_LOOKUP_DEF {
  int value;
  std::string label;

  ENUM_LOOKUP_DEF(int value, const char *enum_name);

  bool operator<(const int &value) const { return this->value < value; }
};

// This acts mainly as a namespace for enum/lookup
class EnumBase {
 public:
  typedef std::string (*lookup_func_t)(int value);

 protected:
  /// @brief Lookup the raw enum 'value' and returns the string description
  /// @brief according to the enum definition (see DECLARE_ENUM). If
  /// @brief the raw value was not defined at compile time returns a plain numeric string
  /// @brief conversion
  static std::string lookup(int value, const ENUM_LOOKUP_DEF *lookup, const ENUM_LOOKUP_DEF *lookup_end);
};

// clang-format off
#define DECLARE_ENUM_ITEM(name, value) name = value
#define DEFINE_ENUM_LOOKUP_ITEM(name, value) {value, #name}

#define DECLARE_ENUM(ENUM_MACRO, underlying_type) \
  class ENUM_MACRO : public EnumBase { \
   public: \
    enum : underlying_type { ENUM_MACRO(DECLARE_ENUM_ITEM) }; \
    static const ENUM_LOOKUP_DEF LOOKUP[]; \
    static const ENUM_LOOKUP_DEF *const LOOKUP_END; \
    inline static std::string lookup(int value) { return EnumBase::lookup(value, LOOKUP, LOOKUP_END); } \
  };

#define DEFINE_ENUM(ENUM_MACRO) \
  const ENUM_LOOKUP_DEF ENUM_MACRO::LOOKUP[] = {ENUM_MACRO(DEFINE_ENUM_LOOKUP_ITEM)}; \
  const ENUM_LOOKUP_DEF *const ENUM_MACRO::LOOKUP_END = ENUM_MACRO::LOOKUP + ARRAY_COUNT(ENUM_MACRO::LOOKUP);

//
// victron BLE enum declarations
//
#define ENUM_VE_REG_AC_IN_ACTIVE(ENUM) \
  ENUM(AC_IN_1, 0x00), ENUM(AC_IN_2, 0x01), ENUM(AC_NOT_CONNECTED, 0x02), ENUM(UNKNOWN, 0x03)
DECLARE_ENUM(ENUM_VE_REG_AC_IN_ACTIVE, u_int8_t)

#define ENUM_VE_REG_ALARM_NOTIFICATION(ENUM) \
  ENUM(NO_ALARM, 0x00), ENUM(WARNING, 0x01), ENUM(ALARM, 0x02), ENUM(UNKNOWN, 0x03)
DECLARE_ENUM(ENUM_VE_REG_ALARM_NOTIFICATION, u_int8_t)

#define ENUM_VE_REG_ALARM_REASON(ENUM) \
  ENUM(NO_ALARM, 0x00), ENUM(LOW_VOLTAGE, 0x01), ENUM(HIGH_VOLTAGE, 0x02), ENUM(LOW_SOC, 0x04), \
  ENUM(LOW_STARTER_VOLTAGE, 0x08), ENUM(HIGH_STARTER_VOLTAGE, 0x10), ENUM(LOW_TEMPERATURE, 0x20), \
  ENUM(HIGH_TEMPERATURE, 0x40), ENUM(MID_VOLTAGE, 0x80), ENUM(OVERLOAD, 0x100), ENUM(DC_RIPPLE, 0x200), \
  ENUM(LOW_V_AC_OUT, 0x400), ENUM(HIGH_V_AC_OUT, 0x800), ENUM(SHORT_CIRCUIT, 0x1000), ENUM(BMS_LOCKOUT, 0x2000), \
  ENUM(UNKNOWN_A, 0x4000), ENUM(UNKNOWN_B, 0x8000)
DECLARE_ENUM(ENUM_VE_REG_ALARM_REASON, u_int16_t)

#define ENUM_VE_REG_BALANCER_STATUS(ENUM) \
  ENUM(NOT_KNOWN, 0x00), ENUM(BALANCED, 0x01), ENUM(BALANCING, 0x02), ENUM(IMBALANCE, 0x03)
DECLARE_ENUM(ENUM_VE_REG_BALANCER_STATUS, u_int8_t)


#define ENUM_VE_REG_CHR_ERROR_CODE(ENUM) \
  ENUM(NO_ERROR, 0), ENUM(BATTERY_TEMPERATURE_HIGH, 1), ENUM(BATTERY_VOLTAGE_HIGH, 2), ENUM(REMOTE_TEMPERATURE_A, 3), \
  ENUM(REMOTE_TEMPERATURE_B, 4), ENUM(REMOTE_TEMPERATURE_C, 5), ENUM(REMOTE_VOLTAGE_A, 6), \
  ENUM(REMOTE_VOLTAGE_B, 7), ENUM(REMOTE_VOLTAGE_C, 8), ENUM(HIGH_RIPPLE, 11), ENUM(BATTERY_TEMPERATURE_LOW, 14), \
  ENUM(CHARGER_TEMPERATURE_HIGH, 17), ENUM(OVER_CURRENT, 18), ENUM(BULK_TIME, 20), ENUM(CURRENT_SENSOR, 21), \
  ENUM(INTERNAL_TEMPERATURE_A, 22), ENUM(INTERNAL_TEMPERATURE_B, 23), ENUM(FAN, 24), ENUM(OVERHEATED, 26), \
  ENUM(SHORT_CIRCUIT, 27), ENUM(CONVERTER_ISSUE, 28), ENUM(OVER_CHARGE, 29), ENUM(INPUT_VOLTAGE, 33), \
  ENUM(INPUT_CURRENT, 34), ENUM(INPUT_POWER, 35), ENUM(INPUT_SHUTDOWN_VOLTAGE, 38), \
  ENUM(INPUT_SHUTDOWN_CURRENT, 39), ENUM(INPUT_SHUTDOWN_FAILURE, 40), ENUM(INVERTER_SHUTDOWN_41, 41), \
  ENUM(INVERTER_SHUTDOWN_42, 42), ENUM(INVERTER_SHUTDOWN_43, 43), ENUM(INVERTER_OVERLOAD, 50), \
  ENUM(INVERTER_TEMPERATURE, 51), ENUM(INVERTER_PEAK_CURRENT, 52), ENUM(INVERTER_OUPUT_VOLTAGE_A, 53), \
  ENUM(INVERTER_OUPUT_VOLTAGE_B, 54), ENUM(INVERTER_SELF_TEST_A, 55), ENUM(INVERTER_SELF_TEST_B, 56), \
  ENUM(INVERTER_AC, 57), ENUM(INVERTER_SELF_TEST_C, 58), ENUM(COMMUNICATION, 65), ENUM(SYNCHRONISATION, 66), \
  ENUM(BMS, 67), ENUM(NETWORK_A, 68), ENUM(NETWORK_B, 69), ENUM(NETWORK_C, 70), ENUM(NETWORK_D, 71), \
  ENUM(PV_INPUT_SHUTDOWN_80, 80), ENUM(PV_INPUT_SHUTDOWN_81, 81), ENUM(PV_INPUT_SHUTDOWN_82, 82), \
  ENUM(PV_INPUT_SHUTDOWN_83, 83), ENUM(PV_INPUT_SHUTDOWN_84, 84), ENUM(PV_INPUT_SHUTDOWN_85, 85), \
  ENUM(PV_INPUT_SHUTDOWN_86, 86), ENUM(PV_INPUT_SHUTDOWN_87, 87), ENUM(CPU_TEMPERATURE_HIGH, 114), \
  ENUM(CALIBRATION_LOST, 116), ENUM(INVALID_FIRMWARE, 117), ENUM(SETTINGS_LOST, 119), ENUM(TESTER_FAIL, 121), \
  ENUM(INTERNAL_DC_VOLTAGE_A, 200), ENUM(INTERNAL_DC_VOLTAGE_B, 201), ENUM(SELF_TEST, 202), \
  ENUM(INTERNAL_SUPPLY_VOLTAGE_A, 203), ENUM(INTERNAL_SUPPLY_VOLTAGE_B, 205), \
  ENUM(INTERNAL_SUPPLY_VOLTAGE_C, 212), ENUM(INTERNAL_SUPPLY_VOLTAGE_D, 215), ENUM(UNKNOWN, 0xFF)
DECLARE_ENUM(ENUM_VE_REG_CHR_ERROR_CODE, u_int8_t)

#define ENUM_VE_REG_DEVICE_OFF_REASON_2(ENUM) \
  ENUM(NOTHING, 0), ENUM(NO_INPUT_POWER, 0x00000001), ENUM(SWITCHED_OFF_SWITCH, 0x00000002), \
  ENUM(SWITCHED_OFF_REGISTER, 0x00000004), ENUM(REMOTE_INPUT, 0x00000008), ENUM(PROTECTION, 0x00000010), \
  ENUM(PAYGO, 0x00000020), ENUM(BMS, 0x00000040), ENUM(ENGINE, 0x00000080), ENUM(INPUT_VOLTATE, 0x00000100),
DECLARE_ENUM(ENUM_VE_REG_DEVICE_OFF_REASON_2, u_int32_t)

#define ENUM_VE_REG_DEVICE_STATE(ENUM) \
  ENUM(OFF, 0x00), ENUM(LOW_POWER, 0x01), ENUM(FAULT, 0x02), ENUM(BULK, 0x03), ENUM(ABSORPTION, 0x04), \
  ENUM(FLOAT, 0x05), ENUM(STORAGE, 0x06), ENUM(EQUALIZE_MANUAL, 0x07), ENUM(PASSTHRU, 0x08), \
  ENUM(INVERTING, 0x09), ENUM(ASSISTING, 0x0A), ENUM(POWER_SUPPLY, 0x0B), ENUM(SUSTAIN, 0xF4), \
  ENUM(STARTING_UP, 0xF5), ENUM(REPEATED_ABSORPTION, 0xF6), ENUM(AUTO_EQUALIZE, 0xF7), ENUM(BATTERY_SAFE, 0xF8), \
  ENUM(EXTERNAL_CONTROL, 0xFC), ENUM(UNKNOWN, 0xFF)
DECLARE_ENUM(ENUM_VE_REG_DEVICE_STATE, u_int8_t)

// This is actually the same encoding as ENUM_VE_REG_ALARM_REASON
typedef ENUM_VE_REG_ALARM_REASON ENUM_VE_REG_WARNING_REASON;

// clang-format on

}  // namespace m3_victron_ble_ir
}  // namespace esphome