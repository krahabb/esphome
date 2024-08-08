import esphome.config_validation as cv
from esphome.components import sensor
from .. import (
    CONF_HEXFRAME,
    CONF_TEXTFRAME,
    HEX_ENTITY_SCHEMA,
    TEXT_ENTITY_SCHEMA,
    m3_vedirect_ns,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# m3_vedirect::Sensor mapped to HEX/TEXT data
CONF_SCALE = "scale"
HFSensor = m3_vedirect_ns.class_("HFSensor", sensor.Sensor)
TFSensor = m3_vedirect_ns.class_("TFSensor", sensor.Sensor)
HFSENSOR_SCHEMA = (
    sensor.sensor_schema(HFSensor)
    .extend(HEX_ENTITY_SCHEMA)
    .extend(
        {
            cv.Optional(CONF_SCALE): cv.float_,
        }
    )
)
TFSENSOR_SCHEMA = (
    sensor.sensor_schema(TFSensor)
    .extend(TEXT_ENTITY_SCHEMA)
    .extend(
        {
            cv.Optional(CONF_SCALE): cv.float_,
        }
    )
)

PLATFORM_ENTITIES = {
    CONF_HEXFRAME: cv.ensure_list(HFSENSOR_SCHEMA),
    CONF_TEXTFRAME: cv.ensure_list(TFSENSOR_SCHEMA),
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def new_tfsensor(config, *args):
    var = await sensor.new_sensor(config, *args)
    if CONF_SCALE in config:
        var.set_scale(config[CONF_SCALE])
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        sensor.new_sensor,
        sensor.new_sensor,
        new_tfsensor,
    )
