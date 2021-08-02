import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome import automation
from esphome.const import (
    CONF_ID,
    CONF_PAYLOAD,
    CONF_REPEAT,
    CONF_TX_PIN,
    CONF_RX_PIN,
    CONF_TRIGGER_ID,
)

CODEOWNERS = ["@krahabb"]

scs_bridge_ns = cg.esphome_ns.namespace("scs_bridge")
SCSBridge = scs_bridge_ns.class_("SCSBridge", cg.Component)
SCSBridgeFrameTrigger = scs_bridge_ns.class_(
    "SCSBridgeFrameTrigger", automation.Trigger.template(cg.std_string)
)
SCSBridgeSendAction = scs_bridge_ns.class_("SCSBridgeSendAction", automation.Action)

CONF_ON_FRAME_RECEIVED = "on_frame_received"
CONF_SCS_COVER_NAME = "scs_cover_name"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SCSBridge),
            cv.Optional(CONF_RX_PIN, default=3): pins.input_pin,
            cv.Optional(CONF_TX_PIN, default=1): pins.output_pin,
            cv.Optional(CONF_SCS_COVER_NAME, default="scs_"): cv.string,
            cv.Optional(CONF_ON_FRAME_RECEIVED): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        SCSBridgeFrameTrigger
                    ),
                }
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    # rx_pin = await cg.pi(config[CONF_RX_PIN])
    # tx_pin = await cg.gpio_pin_expression(config[CONF_TX_PIN])
    # var = cg.new_Pvariable(config[CONF_ID], rx_pin, tx_pin)
    # var = cg.new_Pvariable(config[CONF_ID])
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_RX_PIN],
        config[CONF_TX_PIN],
        config[CONF_SCS_COVER_NAME],
    )
    await cg.register_component(var, config)

    for conf in config.get(CONF_ON_FRAME_RECEIVED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "payload")], conf)


SCSBRIDGE_SEND_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_PAYLOAD): cv.templatable(cv.string),
        cv.Optional(CONF_REPEAT): cv.templatable(cv.uint32_t),
    }
)


@automation.register_action(
    "scs_bridge.send", SCSBridgeSendAction, SCSBRIDGE_SEND_SCHEMA
)
async def scs_bridge_send_to_code(config, action_id, template_args, args):
    # paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_args)
    template_ = await cg.templatable(config[CONF_PAYLOAD], args, cg.std_string)
    cg.add(var.set_payload(template_))
    template_ = await cg.templatable(config[CONF_REPEAT], args, cg.uint32)
    cg.add(var.set_repeat(template_))
    return var
