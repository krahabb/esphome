from esphome.components import select
import esphome.config_validation as cv

from .. import (
    CONF_VEDIRECT_ENTITIES,
    VEDIRECT_ENTITY_SCHEMA,
    m3_vedirect_ns,
    new_vedirect_entity,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# Manager special sensors

# m3_vedirect::Select mapped to HEX/TEXT data
VEDirectSelect = m3_vedirect_ns.class_("Select", select.Select)
VEDIRECT_SELECT_SCHEMA = select.select_schema(VEDirectSelect).extend(
    VEDIRECT_ENTITY_SCHEMA
)

PLATFORM_ENTITIES = {
    CONF_VEDIRECT_ENTITIES: cv.ensure_list(VEDIRECT_SELECT_SCHEMA),
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def new_vedirect_select(config, *args):
    var = await new_vedirect_entity(config, *args)
    await select.register_select(var, config, options=[])
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_select,
        select.new_select,
    )
