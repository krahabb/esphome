from esphome import const as ec
from esphome.components import binary_sensor

from . import PLATFORM_ENTITY_SCHEMA, platform_schema, platform_to_code

_default_schema = {
    PLATFORM_ENTITY_SCHEMA: binary_sensor.binary_sensor_schema(),
}
PLATFORM_ENTITIES = {
    "link_connected": {
        PLATFORM_ENTITY_SCHEMA: binary_sensor.binary_sensor_schema(
            device_class=ec.DEVICE_CLASS_CONNECTIVITY,
        ),
    },
}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config, PLATFORM_ENTITIES, default_init_func=binary_sensor.new_binary_sensor
    )
