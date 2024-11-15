from esphome.components import switch

from .. import (
    VEDIRECT_BINARY_ENTITY_BASE_SCHEMA,
    m3_vedirect_ns,
    new_vedirect_entity,
    ve_reg,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

VEDirectSwitch = m3_vedirect_ns.class_("Switch", switch.Switch)
VEDIRECT_SWITCH_SCHEMA = switch.switch_schema(
    VEDirectSwitch, default_restore_mode="DISABLED"
).extend(VEDIRECT_BINARY_ENTITY_BASE_SCHEMA)


PLATFORM_ENTITIES = {}

CONFIG_SCHEMA = vedirect_platform_schema(
    VEDIRECT_SWITCH_SCHEMA,
    (ve_reg.CLASS.BOOLEAN, ve_reg.CLASS.BITMASK, ve_reg.CLASS.ENUM),
    False,
    PLATFORM_ENTITIES,
)


async def new_vedirect_switch(config, manager):
    var = await new_vedirect_entity(config, manager)
    await switch.register_switch(var, config)
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_switch,
        switch.new_switch,
    )
