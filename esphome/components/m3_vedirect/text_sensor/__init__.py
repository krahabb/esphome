import esphome.config_validation as cv
from esphome.components import text_sensor

from .. import (
    CONF_HEXFRAME,
    CONF_TEXTFRAME,
    HEX_ENTITY_SCHEMA,
    TEXT_ENTITY_SCHEMA,
    m3_vedirect_ns,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# 'Special' text sensors implemented with esphome::text_sensor::TextSensor
_diagnostic_text_sensor_schema = text_sensor.text_sensor_schema(
    entity_category="diagnostic"
)

# m3_vedirect::TextSensor mapped to HEX/TEXT data
HFTextSensor = m3_vedirect_ns.class_("HFTextSensor", text_sensor.TextSensor)
TFTextSensor = m3_vedirect_ns.class_("TFTextSensor", text_sensor.TextSensor)
HEXTEXTSENSOR_SCHEMA = text_sensor.text_sensor_schema(HFTextSensor).extend(
    HEX_ENTITY_SCHEMA
)
TEXTTEXTSENSOR_SCHEMA = text_sensor.text_sensor_schema(TFTextSensor).extend(
    TEXT_ENTITY_SCHEMA
)

PLATFORM_ENTITIES = {
    CONF_HEXFRAME: cv.ensure_list(HEXTEXTSENSOR_SCHEMA),
    CONF_TEXTFRAME: cv.ensure_list(TEXTTEXTSENSOR_SCHEMA),
    "rawhexframe": _diagnostic_text_sensor_schema,
    "rawtextframe": _diagnostic_text_sensor_schema,
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        text_sensor.new_text_sensor,
        text_sensor.new_text_sensor,
        text_sensor.new_text_sensor,
    )
