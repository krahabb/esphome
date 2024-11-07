#pragma once
#include "ve_reg_macro.h"

// clang-format off

/*
	NUMERIC is a numeric TEXT record where the scale matches the one defined in REG_DEF
	this is likely a temporary setting while we migrate the code
*/
#define _REGISTER_TYPE_UNDEFINED TYPE_COUNT
#define TEXTRECORDS(MACRO) \
	IF(DEF_INV)(MACRO(NUMERIC, "AC_OUT_I", AC_OUT_CURRENT, "AC output current")) \
	IF(DEF_INV)(MACRO(NUMERIC, "AC_OUT_S", AC_OUT_APPARENT_POWER, "AC output apparent power")) \
	IF(DEF_INV)(MACRO(NUMERIC, "AC_OUT_V", AC_OUT_VOLTAGE, "AC output voltage")) \
	MACRO(BITMASK, "AR", ALARM_REASON, "Alarm reason") \
	IF(DEF_BMV)(MACRO(BOOLEAN, "Alarm", ALARM_BUZZER, "Alarm")) \
	MACRO(ENUM, "CS", DEVICE_STATE, "State of operation") \
	IF(DEF_CHG)(MACRO(ENUM, "ERR", CHR_ERROR_CODE, "Charger error")) \
	MACRO(STRING, "FW", _REGISTER_TYPE_UNDEFINED, "Firmware version (FW)") \
	MACRO(STRING, "FWE", _REGISTER_TYPE_UNDEFINED, "Firmware version (FWE)") \
	IF(DEF_MPPT)(MACRO(NUMERIC, "H19", USER_YIELD, "Yield total")) \
	IF(DEF_MPPT)(MACRO(NUMERIC, "H20", YIELD_TODAY, "Yield today")) \
	IF(DEF_MPPT)(MACRO(NUMERIC, "H21", MAXIMUM_POWER_TODAY, "Maximum power today")) \
	IF(DEF_MPPT)(MACRO(NUMERIC, "H22", YIELD_YESTERDAY, "Yield yesterday")) \
	IF(DEF_MPPT)(MACRO(NUMERIC, "H23", MAXIMUM_POWER_YESTERDAY, "Maximum power yesterday")) \
	IF(DEF_MPPT)(MACRO(STRING, "HSDS", _REGISTER_TYPE_UNDEFINED, "Day sequence number")) \
	MACRO(NUMERIC, "I", DC_CHANNEL1_CURRENT, "Battery current") \
	IF(DEF_MPPT)(MACRO(NUMERIC, "IL", LOAD_CURRENT, "Load current")) \
	IF(DEF_MPPT)(MACRO(BOOLEAN, "LOAD", LOAD_OUTPUT_STATE, "Output state")) \
	MACRO(ENUM, "MODE", DEVICE_MODE, "Device mode") \
	IF(DEF_BMV71)(MACRO(STRING, "MON", DC_MONITOR_MODE, "DC monitor mode")) \
	IF(DEF_MPPT)(MACRO(ENUM, "MPPT", MPPT_TRACKER_MODE, "Tracker operation mode")) \
	MACRO(BITMASK, "OR", DEVICE_OFF_REASON_2, "Off reason") \
	IF(DEF_BMV)(MACRO(NUMERIC, "P", DC_CHANNEL1_POWER, "Battery power")) \
	MACRO(STRING, "PID", _REGISTER_TYPE_UNDEFINED, "Product Id") \
	IF(DEF_MPPT)(MACRO(NUMERIC, "PPV", PANEL_POWER, "PV power")) \
	MACRO(BOOLEAN, "Relay", RELAY_CONTROL, "Relay state") \
	MACRO(STRING, "SER#", SERIAL_NUMBER, "Serial number") \
	IF(DEF_BMV)(MACRO(NUMERIC, "SOC", SOC, "SOC")) \
	IF(DEF_BMV)(MACRO(NUMERIC, "T", BAT_TEMPERATURE, "Battery temperature")) \
	IF(DEF_BMV)(MACRO(NUMERIC, "TTG", TTG, "Time To Go")) \
	MACRO(NUMERIC, "V", DC_CHANNEL1_VOLTAGE, "Battery voltage") \
	IF(DEF_MPPT)(MACRO(NUMERIC, "VPV", PANEL_VOLTAGE, "PV voltage")) \
	IF(DEF_INV)(MACRO(BITMASK, "WARN", WARNING_REASON, "Warning reason"))

// clang-format on