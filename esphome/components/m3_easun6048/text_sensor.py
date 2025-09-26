from esphome import const as ec
from esphome.components import text_sensor

from .. import m3_easun6048

_PLATFORM = m3_easun6048.Platform(
    text_sensor,
    custom_entity_schema={
        "set_frame": text_sensor.text_sensor_schema(
            entity_category=ec.ENTITY_CATEGORY_DIAGNOSTIC
        ),
        "config_frame": text_sensor.text_sensor_schema(
            entity_category=ec.ENTITY_CATEGORY_DIAGNOSTIC
        ),
        "status_frame": text_sensor.text_sensor_schema(
            entity_category=ec.ENTITY_CATEGORY_DIAGNOSTIC
        ),
    },
)
CONFIG_SCHEMA = _PLATFORM.CONFIG_SCHEMA
to_code = _PLATFORM.to_code
