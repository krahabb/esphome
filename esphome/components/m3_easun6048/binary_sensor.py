from esphome import const as ec
from esphome.components import binary_sensor

from . import Platform

_PLATFORM = Platform(
    binary_sensor,
    custom_entity_schema={
        "link_connected": binary_sensor.binary_sensor_schema(
            device_class=ec.DEVICE_CLASS_CONNECTIVITY,
        ),
    },
)

CONFIG_SCHEMA = _PLATFORM.CONFIG_SCHEMA
to_code = _PLATFORM.to_code
