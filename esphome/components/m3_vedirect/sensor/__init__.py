from esphome.components import sensor
import esphome.const as ec

from .. import (
    m3_vedirect_ns,
    new_vedirect_entity,
    ve_reg,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# m3_vedirect::Sensor mapped to HEX/TEXT data
# CONF_TEXT_SCALE = "text_scale"
# CONF_HEX_SCALE = "hex_scale"
VEDirectSensor = m3_vedirect_ns.class_("Sensor", sensor.Sensor)
VEDIRECT_SENSOR_SCHEMA = sensor.sensor_schema(VEDirectSensor)

PLATFORM_ENTITIES = {
    "run_time": sensor.sensor_schema(
        entity_category="diagnostic",
        device_class=ec.DEVICE_CLASS_DURATION,
        unit_of_measurement=ec.UNIT_SECOND,
    ),
}

CONFIG_SCHEMA = vedirect_platform_schema(
    VEDIRECT_SENSOR_SCHEMA, (ve_reg.CLASS.NUMERIC,), True, PLATFORM_ENTITIES
)


async def new_vedirect_sensor(config, manager):
    var = await new_vedirect_entity(config, manager)
    await sensor.register_sensor(var, config)
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_sensor,
        sensor.new_sensor,
    )
