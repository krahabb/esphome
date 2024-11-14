from esphome.components import binary_sensor
import esphome.config_validation as cv

from .. import (
    CONF_VEDIRECT_ENTITIES,
    VEDIRECT_BINARY_ENTITY_BASE_SCHEMA,
    m3_vedirect_ns,
    new_vedirect_entity,
    ve_reg,
    vedirect_entity_schema,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# Manager special sensors
_diagnostic_binary_sensor_schema = binary_sensor.binary_sensor_schema(
    entity_category="diagnostic"
)


VEDirectBinarySensor = m3_vedirect_ns.class_("BinarySensor", binary_sensor.BinarySensor)
VEDIRECT_BINARY_SENSOR_SCHEMA = binary_sensor.binary_sensor_schema(
    VEDirectBinarySensor
).extend(
    vedirect_entity_schema(
        (ve_reg.CLASS.BOOLEAN, ve_reg.CLASS.BITMASK, ve_reg.CLASS.ENUM), True
    ),
    VEDIRECT_BINARY_ENTITY_BASE_SCHEMA,
)

PLATFORM_ENTITIES = {
    CONF_VEDIRECT_ENTITIES: cv.ensure_list(VEDIRECT_BINARY_SENSOR_SCHEMA),
    "link_connected": _diagnostic_binary_sensor_schema,
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def new_vedirect_binary_sensor(config, manager):
    var = await new_vedirect_entity(config, manager)
    await binary_sensor.register_binary_sensor(var, config)
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_binary_sensor,
        binary_sensor.new_binary_sensor,
    )
