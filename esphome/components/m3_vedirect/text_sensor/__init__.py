from esphome.components import text_sensor
import esphome.config_validation as cv

from .. import (
    CONF_VEDIRECT_ENTITIES,
    VEDIRECT_ENTITY_SCHEMA,
    m3_vedirect_ns,
    new_vedirect_entity,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# 'Special' text sensors implemented with esphome::text_sensor::TextSensor
_diagnostic_text_sensor_schema = text_sensor.text_sensor_schema(
    entity_category="diagnostic"
)

# m3_vedirect::TextSensor mapped to HEX/TEXT data
VEDirectTextSensor = m3_vedirect_ns.class_("TextSensor", text_sensor.TextSensor)
VEDIRECT_TEXT_SENSOR_SCHEMA = text_sensor.text_sensor_schema(VEDirectTextSensor).extend(
    VEDIRECT_ENTITY_SCHEMA
)

PLATFORM_ENTITIES = {
    CONF_VEDIRECT_ENTITIES: cv.ensure_list(VEDIRECT_TEXT_SENSOR_SCHEMA),
    "rawhexframe": _diagnostic_text_sensor_schema,
    "rawtextframe": _diagnostic_text_sensor_schema,
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def new_vedirect_text_sensor(config, *args):
    var = await new_vedirect_entity(config, *args)
    await text_sensor.register_text_sensor(var, config)
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_text_sensor,
        text_sensor.new_text_sensor,
    )
