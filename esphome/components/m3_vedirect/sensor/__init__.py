import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
import esphome.const as ec

from .. import (
    CONF_VEDIRECT_ENTITIES,
    VEDIRECT_ENTITY_SCHEMA,
    m3_vedirect_ns,
    new_vedirect_entity,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# m3_vedirect::Sensor mapped to HEX/TEXT data
CONF_TEXT_SCALE = "text_scale"
CONF_HEX_SCALE = "hex_scale"
VEDirectSensor = m3_vedirect_ns.class_("Sensor", sensor.Sensor)
VEDIRECT_SENSOR_SCHEMA = (
    sensor.sensor_schema(VEDirectSensor)
    .extend(VEDIRECT_ENTITY_SCHEMA)
    .extend(
        {
            cv.Optional(CONF_TEXT_SCALE): cv.float_,
            cv.Optional(CONF_HEX_SCALE): cv.float_,
        }
    )
)

PLATFORM_ENTITIES = {
    CONF_VEDIRECT_ENTITIES: cv.ensure_list(VEDIRECT_SENSOR_SCHEMA),
    "run_time": sensor.sensor_schema(
        entity_category="diagnostic",
        device_class=ec.DEVICE_CLASS_DURATION,
        unit_of_measurement=ec.UNIT_SECOND,
    ),
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def new_vedirect_sensor(config, *args):
    var = await new_vedirect_entity(config, *args)
    if CONF_TEXT_SCALE in config:
        cg.add(var.set_text_scale(config[CONF_TEXT_SCALE]))
    await sensor.register_sensor(var, config)
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_sensor,
        sensor.new_sensor,
    )
