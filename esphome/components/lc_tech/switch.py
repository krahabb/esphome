import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.components import uart

from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
    CONF_CHANNEL,
)

CODEOWNERS = ["@krahabb"]
DEPENDENCIES = ["uart"]

lc_tech_ns = cg.esphome_ns.namespace("lc_tech")
LCTechRelay = lc_tech_ns.class_("LCTechRelay", switch.Switch, cg.PollingComponent)

CONFIG_SCHEMA = (
    switch.SWITCH_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(LCTechRelay),
            cv.Required(CONF_CHANNEL): cv.int_range(1, 4),
        }
    )
    .extend(cv.polling_component_schema("2s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    uart_id = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_id, config[CONF_CHANNEL])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)
