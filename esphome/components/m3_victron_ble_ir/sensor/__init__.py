from esphome.components import sensor

from .. import VBIEntity, m3_victron_ble_ir, platform_schema, platform_to_code

VBISensor = m3_victron_ble_ir.class_("VBISensor", VBIEntity, sensor.Sensor)

_vbisensor_schema = sensor.sensor_schema(VBISensor)

TYPES = [
    "AC_IN_ACTIVE",
    "AC_IN_REAL_POWER",
    "AC_OUT_CURRENT",
    "AC_OUT_VOLTAGE",
    "AC_OUT_APPARENT_POWER",
    "AC_OUT_REAL_POWER",
    "ALARM_NOTIFICATION",
    "ALARM_REASON",
    "BALANCER_STATUS",
    "CHR_ERROR_CODE",
    "CHR_TODAY_YIELD",
    "DC_CHANNEL1_CURRENT",
    "DC_CHANNEL1_VOLTAGE",
    "DC_INPUT_POWER",
    "DEVICE_OFF_REASON_2",
    "DEVICE_STATE",
    "WARNING_REASON",
]
PLATFORM_ENTITIES = {_type: _vbisensor_schema for _type in TYPES}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config,
        PLATFORM_ENTITIES,
        init_func=sensor.new_sensor,
    )
