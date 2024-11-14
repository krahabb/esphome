"""
AUTO GENERATED - See generator.py
"""

import enum
from functools import cached_property

import esphome.codegen as cg

ns = cg.global_ns.namespace("m3_ve_reg")


class MockEnum(enum.IntEnum):
    @cached_property
    def ns(self):
        pass

    @cached_property
    def enum(self):
        return self.ns.enum(self.name)


HEXFRAME_struct = ns.struct("HEXFRAME")


class COMMAND(MockEnum):
    Ping = enum.auto()
    Done = enum.auto()
    AppVersion = enum.auto()
    Unknown = enum.auto()
    ProductId = enum.auto()
    Error = enum.auto()
    PingResp = enum.auto()
    Restart = enum.auto()
    Get = enum.auto()
    Set = enum.auto()
    Async = enum.auto()

    @cached_property
    def ns(self):
        return HEXFRAME_struct.enum("COMMAND", is_class=True)


class DATA_TYPE(MockEnum):
    VARIADIC = enum.auto()
    UN8 = enum.auto()
    UN16 = enum.auto()
    UN32 = enum.auto()
    SN8 = enum.auto()
    SN16 = enum.auto()
    SN32 = enum.auto()
    _COUNT = enum.auto()

    @cached_property
    def ns(self):
        return HEXFRAME_struct.enum("DATA_TYPE", is_class=True)


REG_DEF_struct = ns.struct("REG_DEF")


class CLASS(MockEnum):
    UNKNOWN = enum.auto()
    BITMASK = enum.auto()
    BOOLEAN = enum.auto()
    ENUM = enum.auto()
    NUMERIC = enum.auto()
    STRING = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("CLASS", is_class=True)


class ACCESS(MockEnum):
    READ_ONLY = enum.auto()
    READ_WRITE = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("ACCESS", is_class=True)


class UNIT(MockEnum):
    NONE = enum.auto()
    A = enum.auto()
    V = enum.auto()
    VA = enum.auto()
    W = enum.auto()
    Ah = enum.auto()
    kWh = enum.auto()
    SOC_PERCENTAGE = enum.auto()
    minute = enum.auto()
    CELSIUS = enum.auto()
    UNIT_COUNT = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("UNIT", is_class=True)


class SCALE(MockEnum):
    S_1 = enum.auto()
    S_0_1 = enum.auto()
    S_0_01 = enum.auto()
    S_0_001 = enum.auto()
    S_0_25 = enum.auto()
    SCALE_COUNT = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("SCALE", is_class=True)


class Flavor(enum.StrEnum):
    ALL = enum.auto()
    INV_PHNX = enum.auto()
    CHG_PHNX = enum.auto()
    MPPT_BS = enum.auto()
    MPPT_RS = enum.auto()
    BMV60 = enum.auto()
    BMV70 = enum.auto()
    BMV71 = enum.auto()
    MPPT = enum.auto()
    BMV = enum.auto()
    CHG = enum.auto()
    INV = enum.auto()


class TYPE(MockEnum):
    UNDEFINED = enum.auto()
    BLE_MODE = enum.auto()
    PRODUCT_ID = enum.auto()
    SERIAL_NUMBER = enum.auto()
    MODEL_NAME = enum.auto()
    CAPABILITIES = enum.auto()
    CAPABILITIES_BLE = enum.auto()
    DEVICE_MODE = enum.auto()
    DEVICE_STATE = enum.auto()
    DEVICE_OFF_REASON = enum.auto()
    DEVICE_OFF_REASON_2 = enum.auto()
    AC_OUT_VOLTAGE_SETPOINT = enum.auto()
    WARNING_REASON = enum.auto()
    ALARM_REASON = enum.auto()
    ALARM_LOW_VOLTAGE_SET = enum.auto()
    ALARM_LOW_VOLTAGE_CLEAR = enum.auto()
    RELAY_CONTROL = enum.auto()
    RELAY_MODE = enum.auto()
    TTG = enum.auto()
    SOC = enum.auto()
    AC_OUT_VOLTAGE = enum.auto()
    AC_OUT_CURRENT = enum.auto()
    AC_OUT_APPARENT_POWER = enum.auto()
    SHUTDOWN_LOW_VOLTAGE_SET = enum.auto()
    VOLTAGE_RANGE_MIN = enum.auto()
    VOLTAGE_RANGE_MAX = enum.auto()
    DC_CHANNEL1_VOLTAGE = enum.auto()
    DC_CHANNEL1_POWER = enum.auto()
    DC_CHANNEL1_CURRENT = enum.auto()
    LOAD_OUTPUT_STATE = enum.auto()
    LOAD_CURRENT = enum.auto()
    MPPT_TRACKER_MODE = enum.auto()
    PANEL_MAXIMUM_VOLTAGE = enum.auto()
    PANEL_VOLTAGE = enum.auto()
    PANEL_POWER = enum.auto()
    PANEL_CURRENT = enum.auto()
    MAXIMUM_POWER_YESTERDAY = enum.auto()
    YIELD_YESTERDAY = enum.auto()
    MAXIMUM_POWER_TODAY = enum.auto()
    YIELD_TODAY = enum.auto()
    CHR_ERROR_CODE = enum.auto()
    USER_YIELD = enum.auto()
    SYSTEM_YIELD = enum.auto()
    BAT_TEMPERATURE = enum.auto()
    DC_MONITOR_MODE = enum.auto()
    ALARM_BUZZER = enum.auto()

    @cached_property
    def ns(self):
        return REG_DEF_struct.enum("TYPE", is_class=True)
