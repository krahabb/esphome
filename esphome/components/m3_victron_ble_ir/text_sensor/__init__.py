from esphome.components import text_sensor

from .. import (
    VBIEntity,
    VBIEntity_CLASS_ENUM,
    VBIEntity_TYPES,
    m3_victron_ble_ir,
    platform_schema,
    platform_to_code,
)

VBITextSensor = m3_victron_ble_ir.class_(
    "VBITextSensor", VBIEntity, text_sensor.TextSensor
)

_vbitextsensor_schema = text_sensor.text_sensor_schema(VBITextSensor)

PLATFORM_VBI_ENTITIES = {
    _type: _vbitextsensor_schema
    for _type, _class in VBIEntity_TYPES.items()
    if _class is VBIEntity_CLASS_ENUM
}

CONFIG_SCHEMA = platform_schema(PLATFORM_VBI_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config,
        PLATFORM_VBI_ENTITIES,
        init_func=text_sensor.new_text_sensor,
    )
