from esphome.components import text_sensor

from . import PLATFORM_ENTITY_SCHEMA, platform_schema, platform_to_code

_default_schema = {
    PLATFORM_ENTITY_SCHEMA: text_sensor.text_sensor_schema(),
}
PLATFORM_ENTITIES = {
    "charge_mos_status": _default_schema,
    "discharge_mos_status": _default_schema,
    "balance_status": _default_schema,
}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config, PLATFORM_ENTITIES, default_init_func=text_sensor.new_text_sensor
    )
