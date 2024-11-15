from esphome.components import text_sensor

from .. import (
    m3_vedirect_ns,
    new_vedirect_entity,
    ve_reg,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# 'Special' text sensors implemented with esphome::text_sensor::TextSensor
_diagnostic_text_sensor_schema = text_sensor.text_sensor_schema(
    entity_category="diagnostic"
)

# m3_vedirect::TextSensor mapped to HEX/TEXT data
VEDirectTextSensor = m3_vedirect_ns.class_("TextSensor", text_sensor.TextSensor)
VEDIRECT_TEXT_SENSOR_SCHEMA = text_sensor.text_sensor_schema(VEDirectTextSensor)
PLATFORM_ENTITIES = {
    "rawhexframe": _diagnostic_text_sensor_schema,
    "rawtextframe": _diagnostic_text_sensor_schema,
}

CONFIG_SCHEMA = vedirect_platform_schema(
    VEDIRECT_TEXT_SENSOR_SCHEMA,
    (ve_reg.CLASS.BITMASK, ve_reg.CLASS.ENUM, ve_reg.CLASS.STRING),
    True,
    PLATFORM_ENTITIES,
)


async def new_vedirect_text_sensor(config, manager):
    var = await new_vedirect_entity(config, manager)
    await text_sensor.register_text_sensor(var, config)
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_text_sensor,
        text_sensor.new_text_sensor,
    )
