from esphome.components import select

from .. import (
    m3_vedirect_ns,
    new_vedirect_entity,
    ve_reg,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

# Manager special sensors

# m3_vedirect::Select mapped to HEX/TEXT data
VEDirectSelect = m3_vedirect_ns.class_("Select", select.Select)
VEDIRECT_SELECT_SCHEMA = select.select_schema(VEDirectSelect)

PLATFORM_ENTITIES = {}
CONFIG_SCHEMA = vedirect_platform_schema(
    VEDIRECT_SELECT_SCHEMA, (ve_reg.CLASS.ENUM,), False, PLATFORM_ENTITIES
)


async def new_vedirect_select(config, manager):
    var = await new_vedirect_entity(config, manager)
    await select.register_select(var, config, options=[])
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_select,
        select.new_select,
    )
