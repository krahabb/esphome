from esphome import automation
import esphome.codegen as cg
from esphome.components import esp32_ble_tracker
import esphome.config_validation as cv
from esphome.const import (
    CONF_BINDKEY,
    CONF_ID,
    CONF_MAC_ADDRESS,
    CONF_ON_MESSAGE,
    CONF_TRIGGER_ID,
)
from esphome.yaml_util import ESPHomeDumper

CODEOWNERS = ["@krahabb"]
DEPENDENCIES = ["esp32_ble_tracker"]
MULTI_CONF = True

CONF_VICTRON_BLE_IR_ID = "victron_ble_ir_id"
CONF_AUTO_CREATE_ENTITIES = "auto_create_entities"

m3_victron_ble_ir = cg.esphome_ns.namespace("m3_victron_ble_ir")
Manager = m3_victron_ble_ir.class_(
    "Manager", esp32_ble_tracker.ESPBTDeviceListener, cg.Component
)
VBIEntity = m3_victron_ble_ir.class_("VBIEntity")
VBIEntity_Type = VBIEntity.enum("TYPE")

PLATFORM_ENTITY_SCHEMA = "schema"
PLATFORM_ENTITY_INIT = "init"

# root schema to group (platform) entities linked to a Victron BLE device
PLATFORM_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_VICTRON_BLE_IR_ID): cv.use_id(Manager),
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
    init_func,
):
    manager = await cg.get_variable(config[CONF_VICTRON_BLE_IR_ID])

    for entity_key, entity_config in config.items():
        if entity_key in platform_entities:
            entity = await init_func(entity_config, VBIEntity_Type.enum(entity_key))
            cg.add(getattr(manager, "register_entity")(entity))


VictronBleRecordConstPtr = (
    m3_victron_ble_ir.struct("VICTRON_BLE_RECORD").operator("ptr").operator("const")
)
MessageTrigger = m3_victron_ble_ir.class_(
    "MessageTrigger", automation.Trigger.template(VictronBleRecordConstPtr)
)


class Array:
    def __init__(self, *parts):
        self.parts = parts

    def __str__(self):
        return "".join(f"{part:02X}" for part in self.parts)

    @property
    def as_array(self):
        from esphome.cpp_generator import RawExpression

        num = ", 0x".join(f"{part:02X}" for part in self.parts)
        return RawExpression(f"{{0x{num}}}")


ESPHomeDumper.add_multi_representer(Array, ESPHomeDumper.represent_stringify)


def bind_key_array(value):
    value = cv.bind_key(value)
    parts = [value[i : i + 2] for i in range(0, len(value), 2)]
    parts_int = []
    for part in parts:
        parts_int.append(int(part, 16))
    return Array(*parts_int)


def bind_mac_address_or_shortened(value):
    value = cv.string_strict(value)
    if ":" in value:
        return cv.mac_address(value)
    if len(value) != 12:
        raise cv.Invalid("MAC Address must be format XX:XX:XX:XX:XX:XX or XXXXXXXXXXXX")
    # Split every second character
    n = 2
    parts_int = [value[i : i + n] for i in range(0, len(value), n)]
    return cv.mac_address(":".join(parts_int))


CONFIG_SCHEMA = cv.All(
    cv.only_on_esp32,
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Manager),
            cv.Required(CONF_MAC_ADDRESS): bind_mac_address_or_shortened,
            cv.Required(CONF_BINDKEY): bind_key_array,
            cv.Optional(CONF_AUTO_CREATE_ENTITIES, default=False): cv.boolean,
            cv.Optional(CONF_ON_MESSAGE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MessageTrigger),
                }
            ),
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)

    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
    cg.add(var.set_bindkey(config[CONF_BINDKEY].as_array))
    cg.add(var.set_auto_create_entities(config[CONF_AUTO_CREATE_ENTITIES]))

    for conf in config.get(CONF_ON_MESSAGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(
            trigger, [(VictronBleRecordConstPtr, "message")], conf
        )
