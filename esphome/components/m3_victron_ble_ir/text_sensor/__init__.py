from esphome.components import text_sensor

from .. import VBIEntity, m3_victron_ble_ir, platform_schema, platform_to_code

VBITextSensor = m3_victron_ble_ir.class_(
    "VBITextSensor", text_sensor.TextSensor, VBIEntity
)

_vbitextsensor_schema = text_sensor.text_sensor_schema(VBITextSensor)

TYPES = [
    "AC_IN_ACTIVE",
    "ALARM_NOTIFICATION",
    "ALARM_REASON",
    "BALANCER_STATUS",
    "CHR_ERROR_CODE",
    "DEVICE_OFF_REASON_2",
    "DEVICE_STATE",
    "WARNING_REASON",
]
PLATFORM_ENTITIES = {_type: _vbitextsensor_schema for _type in TYPES}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config,
        PLATFORM_ENTITIES,
        init_func=text_sensor.new_text_sensor,
    )
