import esphome.config_validation as cv
from esphome.components import binary_sensor
from .. import (
    CONF_HEXFRAME,
    CONF_TEXTFRAME,
    HEX_ENTITY_SCHEMA,
    TEXT_ENTITY_SCHEMA,
    m3_vedirect_ns,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# Manager special sensors
_diagnostic_binary_sensor_schema = binary_sensor.binary_sensor_schema(
    entity_category="diagnostic"
)

# m3_vedirect::BinarySensor mapped to HEX/TEXT data
HFBinarySensor = m3_vedirect_ns.class_("HFBinarySensor", binary_sensor.BinarySensor)
TFBinarySensor = m3_vedirect_ns.class_("TFBinarySensor", binary_sensor.BinarySensor)
HFBINARYSENSOR_SCHEMA = binary_sensor.binary_sensor_schema(HFBinarySensor).extend(
    HEX_ENTITY_SCHEMA
)
TFBINARYSENSOR_SCHEMA = binary_sensor.binary_sensor_schema(TFBinarySensor).extend(
    TEXT_ENTITY_SCHEMA
)

PLATFORM_ENTITIES = {
    CONF_HEXFRAME: cv.ensure_list(HFBINARYSENSOR_SCHEMA),
    CONF_TEXTFRAME: cv.ensure_list(TFBINARYSENSOR_SCHEMA),
    "link_connected": _diagnostic_binary_sensor_schema,
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        binary_sensor.new_binary_sensor,
        binary_sensor.new_binary_sensor,
        binary_sensor.new_binary_sensor,
    )
