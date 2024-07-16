import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_ID,
    CONF_ADDRESS,
)

scs_bridge_ns = cg.esphome_ns.namespace("scs_bridge")
SCSSwitch = scs_bridge_ns.class_("SCSSwitch", switch.Switch)

CONFIG_SCHEMA = switch.SWITCH_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(SCSSwitch),
        cv.Required(CONF_ADDRESS): cv.hex_uint8_t,
    }
)  # .extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_address(config[CONF_ADDRESS]))
    await switch.register_switch(var, config)
