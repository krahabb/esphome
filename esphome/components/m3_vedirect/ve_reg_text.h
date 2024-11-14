#pragma once
#include "ve_reg_flavor.h"

// clang-format off

/*
	NUMERIC is a numeric TEXT record where the scale matches the one defined in REG_DEF
	this is likely a temporary setting while we migrate the code
*/
#define _REGISTER_TYPE_UNDEFINED TYPE_COUNT
#define TEXTRECORDS(MACRO) \
	MACRO(INV, NUMERIC, "AC_OUT_I", AC_OUT_CURRENT, "AC output current") \
	MACRO(INV, NUMERIC, "AC_OUT_S", AC_OUT_APPARENT_POWER, "AC output apparent power") \
	MACRO(INV, NUMERIC, "AC_OUT_V", AC_OUT_VOLTAGE, "AC output voltage") \
	MACRO(ANY, BITMASK, "AR", ALARM_REASON, "Alarm reason") \
	MACRO(BMV, BOOLEAN, "Alarm", ALARM_BUZZER, "Alarm") \
	MACRO(ANY, ENUM, "CS", DEVICE_STATE, "State of operation") \
	MACRO(CHG, ENUM, "ERR", CHR_ERROR_CODE, "Charger error") \
	MACRO(ANY, STRING, "FW", _REGISTER_TYPE_UNDEFINED, "Firmware version (FW)") \
	MACRO(ANY, STRING, "FWE", _REGISTER_TYPE_UNDEFINED, "Firmware version (FWE)") \
	MACRO(MPPT, NUMERIC, "H19", USER_YIELD, "Yield total") \
	MACRO(MPPT, NUMERIC, "H20", YIELD_TODAY, "Yield today") \
	MACRO(MPPT, NUMERIC, "H21", MAXIMUM_POWER_TODAY, "Maximum power today") \
	MACRO(MPPT, NUMERIC, "H22", YIELD_YESTERDAY, "Yield yesterday") \
	MACRO(MPPT, NUMERIC, "H23", MAXIMUM_POWER_YESTERDAY, "Maximum power yesterday") \
	MACRO(MPPT, STRING, "HSDS", _REGISTER_TYPE_UNDEFINED, "Day sequence number") \
	MACRO(ANY, NUMERIC, "I", DC_CHANNEL1_CURRENT, "Battery current") \
	MACRO(MPPT, NUMERIC, "IL", LOAD_CURRENT, "Load current") \
	MACRO(MPPT, BOOLEAN, "LOAD", LOAD_OUTPUT_STATE, "Output state") \
	MACRO(ANY, ENUM, "MODE", DEVICE_MODE, "Device mode") \
	MACRO(BMV71, STRING, "MON", DC_MONITOR_MODE, "DC monitor mode") \
	MACRO(MPPT, ENUM, "MPPT", MPPT_TRACKER_MODE, "Tracker operation mode") \
	MACRO(ANY, BITMASK, "OR", DEVICE_OFF_REASON_2, "Off reason") \
	MACRO(BMV, NUMERIC, "P", DC_CHANNEL1_POWER, "Battery power") \
	MACRO(ANY, UNKNOWN, "PID", PRODUCT_ID, "Product Id") \
	MACRO(MPPT, NUMERIC, "PPV", PANEL_POWER, "PV power") \
	MACRO(ANY, BOOLEAN, "Relay", RELAY_CONTROL, "Relay state") \
	MACRO(ANY, STRING, "SER#", SERIAL_NUMBER, "Serial number") \
	MACRO(BMV, NUMERIC, "SOC", SOC, "SOC") \
	MACRO(BMV, NUMERIC, "T", BAT_TEMPERATURE, "Battery temperature") \
	MACRO(BMV, NUMERIC, "TTG", TTG, "Time To Go") \
	MACRO(ANY, NUMERIC, "V", DC_CHANNEL1_VOLTAGE, "Battery voltage") \
	MACRO(MPPT, NUMERIC, "VPV", PANEL_VOLTAGE, "PV voltage") \
	MACRO(INV, BITMASK, "WARN", WARNING_REASON, "Warning reason")

// clang-format on