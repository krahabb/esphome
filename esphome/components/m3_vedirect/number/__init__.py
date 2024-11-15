from esphome.components import number
import esphome.config_validation as cv
import esphome.const as ec

from .. import (
    m3_vedirect_ns,
    new_vedirect_entity,
    ve_reg,
    vedirect_platform_schema,
    vedirect_platform_to_code,
)

VEDirectNumber = m3_vedirect_ns.class_("Number", number.Number)
VEDIRECT_NUMBER_SCHEMA = number.number_schema(VEDirectNumber).extend(
    {
        cv.Required(ec.CONF_MIN_VALUE): cv.float_,
        cv.Required(ec.CONF_MAX_VALUE): cv.float_,
        cv.Required(ec.CONF_STEP): cv.positive_float,
    }
)

PLATFORM_ENTITIES = {}
CONFIG_SCHEMA = vedirect_platform_schema(
    VEDIRECT_NUMBER_SCHEMA, (ve_reg.CLASS.NUMERIC,), False, PLATFORM_ENTITIES
)


async def new_vedirect_number(config, manager):
    var = await new_vedirect_entity(config, manager)
    await number.register_number(
        var,
        config,
        min_value=config[ec.CONF_MIN_VALUE],
        max_value=config[ec.CONF_MAX_VALUE],
        step=config[ec.CONF_STEP],
    )
    return var


async def to_code(config: dict):
    await vedirect_platform_to_code(
        config,
        PLATFORM_ENTITIES,
        new_vedirect_number,
        number.new_number,
    )
