import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_ADDRESS,
    CONF_MAX_DURATION,
)

scs_bridge_ns = cg.esphome_ns.namespace("scs_bridge")
SCSCover = scs_bridge_ns.class_("SCSCover", cover.Cover)

CONFIG_SCHEMA = cover.COVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(SCSCover),
        cv.Required(CONF_ADDRESS): cv.hex_uint8_t,
        cv.Optional(CONF_MAX_DURATION): cv.positive_time_period_seconds,
    }
)  # .extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_ADDRESS], config[CONF_NAME])

    await cover.register_cover(var, config)

    if CONF_MAX_DURATION in config:
        cg.add(var.set_max_duration(config[CONF_MAX_DURATION]))
