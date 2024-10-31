import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv

from .. import (
    CONF_VEDIRECT_ENTITIES,
    VEDIRECT_ENTITY_SCHEMA,
    m3_vedirect_ns,
    new_vedirect_entity,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

CONF_MASK = "mask"
VEDirectSwitch = m3_vedirect_ns.class_("Switch", switch.Switch)
VEDIRECT_SWITCH_SCHEMA = (
    switch.switch_schema(
        VEDirectSwitch,
        default_restore_mode="DISABLED",
    )
    .extend(VEDIRECT_ENTITY_SCHEMA)
    .extend(
        {
            cv.Optional(CONF_MASK): cv.uint32_t,
        }
    )
)


PLATFORM_ENTITIES = {
    CONF_VEDIRECT_ENTITIES: cv.ensure_list(VEDIRECT_SWITCH_SCHEMA),
}

CONFIG_SCHEMA = vedirect_platform_schema(PLATFORM_ENTITIES)


async def new_vedirect_switch(config, manager):
    var = await new_vedirect_entity(config, manager)
    if CONF_MASK in config:
        cg.add(var.set_mask(config[CONF_MASK]))
    await switch.register_switch(var, config)
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_switch,
        switch.new_switch,
    )
