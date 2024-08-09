import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@krahabb"]
AUTO_LOAD = ["binary_sensor", "text_sensor", "sensor", "switch", "output"]
MULTI_CONF = True

CONF_ANTBMS_ID = "antbms_id"

m3_ant_bms_ns = cg.esphome_ns.namespace("m3_antbms")
AntBmsComponent = m3_ant_bms_ns.class_("AntBms", cg.PollingComponent)

# root schema to group (platform) entities linked to a Bms
PLATFORM_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ANTBMS_ID): cv.use_id(AntBmsComponent),
    }
)


def platform_schema(
    platform_entities: dict[str, cv.Schema],
):
    return PLATFORM_SCHEMA.extend(
        {cv.Optional(type): schema for type, schema in platform_entities.items()}
    )


async def platform_to_code(
    config: dict,
    platform_entities: dict[str, cv.Schema],
    setup_entity_func,
):
    antbms = await cg.get_variable(config[CONF_ANTBMS_ID])

    for entity_key, entity_config in config.items():
        if entity_key in platform_entities:
            var = await setup_entity_func(entity_config)
            cg.add(getattr(antbms, f"set_{entity_key}")(var))


CONFIG_SCHEMA = cv.All(
    cv.Schema({cv.GenerateID(): cv.declare_id(AntBmsComponent)})
    .extend(cv.polling_component_schema("1s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
