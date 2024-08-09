from esphome import const as ec
from esphome.components import sensor

from . import platform_schema, platform_to_code

DEPENDENCIES = ["uart"]

PLATFORM_ENTITIES = {
    "battery_voltage": sensor.sensor_schema(
        unit_of_measurement=ec.UNIT_VOLT,
        accuracy_decimals=1,
        device_class=ec.DEVICE_CLASS_VOLTAGE,
        state_class=ec.STATE_CLASS_MEASUREMENT,
    ),
    "battery_current": sensor.sensor_schema(
        unit_of_measurement=ec.UNIT_AMPERE,
        accuracy_decimals=1,
        device_class=ec.DEVICE_CLASS_CURRENT,
        state_class=ec.STATE_CLASS_MEASUREMENT,
    ),
    "soc": sensor.sensor_schema(
        unit_of_measurement=ec.UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=ec.DEVICE_CLASS_BATTERY,
        state_class=ec.STATE_CLASS_MEASUREMENT,
    ),
}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(config, PLATFORM_ENTITIES, sensor.new_sensor)
