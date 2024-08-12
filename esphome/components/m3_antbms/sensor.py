from esphome import config_validation as cv, const as ec
from esphome.components import sensor

from . import CONF_COUNT, PLATFORM_ENTITY_SCHEMA, platform_schema, platform_to_code

PLATFORM_ENTITIES = {
    "battery_voltage": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_VOLT,
            accuracy_decimals=1,
            device_class=ec.DEVICE_CLASS_VOLTAGE,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ),
    },
    "battery_current": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_AMPERE,
            accuracy_decimals=1,
            device_class=ec.DEVICE_CLASS_CURRENT,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ),
    },
    "battery_power": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_WATT,
            accuracy_decimals=0,
            device_class=ec.DEVICE_CLASS_POWER,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ),
    },
    "soc": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=ec.DEVICE_CLASS_BATTERY,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ),
    },
    "capacity_remaining": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement="Ah",
            accuracy_decimals=0,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ),
    },
    "cell_voltage": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_VOLT,
            accuracy_decimals=2,
            device_class=ec.DEVICE_CLASS_VOLTAGE,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ).extend(
            {
                cv.Required(CONF_COUNT): cv.int_range(1, 32),
            }
        ),
    },
    "cell_high_voltage": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_VOLT,
            accuracy_decimals=2,
            device_class=ec.DEVICE_CLASS_VOLTAGE,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ),
    },
    "cell_low_voltage": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_VOLT,
            accuracy_decimals=2,
            device_class=ec.DEVICE_CLASS_VOLTAGE,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ),
    },
    "cell_count": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            accuracy_decimals=0,
        ),
    },
    "temperature": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement=ec.UNIT_CELSIUS,
            accuracy_decimals=0,
            device_class=ec.DEVICE_CLASS_TEMPERATURE,
            state_class=ec.STATE_CLASS_MEASUREMENT,
        ).extend(
            {
                cv.Required(CONF_COUNT): cv.int_range(1, 6),
            }
        ),
    },
    # miscellaneous/diagnostic entities
    "memory_free": {
        PLATFORM_ENTITY_SCHEMA: sensor.sensor_schema(
            unit_of_measurement="kb",
            accuracy_decimals=0,
            state_class=ec.STATE_CLASS_MEASUREMENT,
            entity_category=ec.ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    },
}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config, PLATFORM_ENTITIES, default_init_func=sensor.new_sensor
    )
