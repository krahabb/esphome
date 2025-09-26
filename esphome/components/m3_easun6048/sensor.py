from esphome.components import sensor

from .. import m3_easun6048

_PLATFORM = m3_easun6048.Platform(
    sensor,
    frame_entity_schema=m3_easun6048.NUMERIC_FRAME_ENTITY_SCHEMA,
)
CONFIG_SCHEMA = _PLATFORM.CONFIG_SCHEMA

to_code = _PLATFORM.to_code
