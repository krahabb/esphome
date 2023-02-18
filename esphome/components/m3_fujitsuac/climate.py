from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_TX_PIN, CONF_ADDRESS
from esphome.components import climate
from esphome.components import uart

fujitsu_ns = cg.esphome_ns.namespace("m3_fujitsuac")
FujitsuClimateComponent = fujitsu_ns.class_(
    "FujitsuClimate", climate.Climate, uart.UARTDevice, cg.Component
)

CONFIG_SCHEMA = (
    climate.CLIMATE_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(FujitsuClimateComponent),
            cv.Required(CONF_ADDRESS): cv.hex_int_range(min=32, max=33),
            cv.Optional(CONF_TX_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

"""CONFIG_SCHEMA = cv.All(
    cv.Schema({cv.GenerateID(): cv.declare_id(FujitsuClimateComponent)})
    .extend(climate.CLIMATE_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)"""


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_address(config[CONF_ADDRESS]))
    if CONF_TX_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_TX_PIN])
        cg.add(var.set_txenable_pin(pin))
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await climate.register_climate(var, config)
