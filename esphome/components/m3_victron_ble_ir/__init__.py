from esphome import automation, codegen as cg, config_validation as cv, const as ec
from esphome.components import esp32_ble_tracker
from esphome.core import CORE
from esphome.yaml_util import ESPHomeDumper

CODEOWNERS = ["@krahabb"]
DEPENDENCIES = ["esp32_ble_tracker"]
AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]
MULTI_CONF = True

ESPHOME_VERSION = cv.Version.parse(ec.__version__)

CONF_VICTRON_BLE_IR_ID = "victron_ble_ir_id"
CONF_AUTO_CREATE_ENTITIES = "auto_create_entities"
CONF_LINK_CONNECTED_TIMEOUT = "link_connected_timeout"

m3_victron_ble_ir = cg.esphome_ns.namespace("m3_victron_ble_ir")
Manager = m3_victron_ble_ir.class_(
    "Manager", esp32_ble_tracker.ESPBTDeviceListener, cg.Component
)

VBIRecord = m3_victron_ble_ir.struct("VBI_RECORD")
VBIRecordConstPtr = VBIRecord.operator("ptr").operator("const")

VBIRecord_HEADER = VBIRecord.struct("HEADER")
VBIRecord_TYPE = VBIRecord_HEADER.enum("TYPE")

MessageTrigger = m3_victron_ble_ir.class_(
    "MessageTrigger", automation.Trigger.template(VBIRecordConstPtr)
)
VBIRecord_TYPES = [
    "TEST_RECORD",
    "SOLAR_CHARGER",
    "BATTERY_MONITOR",
    "INVERTER",
    "DCDC_CONVERTER",
    "SMART_LITHIUM",
    "INVERTER_RS",
    "GX_DEVICE",
    "AC_CHARGER",
    "SMART_BATTERY_PROTECT",
    "LYNX_SMART_BMS",
    "MULTI_RS",
    "VE_BUS",
    "DC_ENERGY_METER",
    "ORION_XS",
    "AUTO",
]
VBIEntity = m3_victron_ble_ir.class_("VBIEntity")
VBIEntity_TYPE = VBIEntity.enum("TYPE")
VBIEntity_CLASS = VBIEntity.enum("CLASS")
VBIEntity_CLASS_ENUM = VBIEntity_CLASS.enum("ENUM")
VBIEntity_CLASS_MEASURE = VBIEntity_CLASS.enum("MEASURE")

VBIEntity_TYPES = {
    "AC_IN_ACTIVE": VBIEntity_CLASS_ENUM,
    "AC_IN_REAL_POWER": VBIEntity_CLASS_MEASURE,
    "AC_IN_1_REAL_POWER": VBIEntity_CLASS_MEASURE,
    "AC_IN_2_REAL_POWER": VBIEntity_CLASS_MEASURE,
    "AC_OUT_CURRENT": VBIEntity_CLASS_MEASURE,
    "AC_OUT_VOLTAGE": VBIEntity_CLASS_MEASURE,
    "AC_OUT_APPARENT_POWER": VBIEntity_CLASS_MEASURE,
    "AC_OUT_REAL_POWER": VBIEntity_CLASS_MEASURE,
    "ALARM_NOTIFICATION": VBIEntity_CLASS_ENUM,
    "ALARM_REASON": VBIEntity_CLASS_ENUM,
    "BALANCER_STATUS": VBIEntity_CLASS_ENUM,
    "BAT_TEMPERATURE": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_1": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_2": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_3": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_4": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_5": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_6": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_7": VBIEntity_CLASS_MEASURE,
    "BATTERY_CELL_VOLTAGE_8": VBIEntity_CLASS_MEASURE,
    "BATTERY_MID_POINT_VOLTAGE": VBIEntity_CLASS_MEASURE,
    "BMS_ERROR": VBIEntity_CLASS_ENUM,
    "BMS_FLAGS": VBIEntity_CLASS_ENUM,
    "BMV_AUX_INPUT": VBIEntity_CLASS_ENUM,
    "BMV_MONITOR_MODE": VBIEntity_CLASS_ENUM,
    "CAH": VBIEntity_CLASS_MEASURE,
    "CHR_ERROR_CODE": VBIEntity_CLASS_ENUM,
    "CHR_TODAY_YIELD": VBIEntity_CLASS_MEASURE,
    "DC_CHANNEL1_CURRENT": VBIEntity_CLASS_MEASURE,
    "DC_CHANNEL1_POWER": VBIEntity_CLASS_MEASURE,
    "DC_CHANNEL1_VOLTAGE": VBIEntity_CLASS_MEASURE,
    "DC_CHANNEL2_CURRENT": VBIEntity_CLASS_MEASURE,
    "DC_CHANNEL2_VOLTAGE": VBIEntity_CLASS_MEASURE,
    "DC_CHANNEL3_CURRENT": VBIEntity_CLASS_MEASURE,
    "DC_CHANNEL3_VOLTAGE": VBIEntity_CLASS_MEASURE,
    "DC_INPUT_VOLTAGE": VBIEntity_CLASS_MEASURE,
    "DC_INPUT_POWER": VBIEntity_CLASS_MEASURE,
    "DC_OUTPUT_CURRENT": VBIEntity_CLASS_MEASURE,
    "DC_OUTPUT_STATUS": VBIEntity_CLASS_ENUM,
    "DC_OUTPUT_VOLTAGE": VBIEntity_CLASS_MEASURE,
    "DEVICE_OFF_REASON_2": VBIEntity_CLASS_ENUM,
    "DEVICE_STATE": VBIEntity_CLASS_ENUM,
    "SMART_LITHIUM_ERROR_FLAGS": VBIEntity_CLASS_ENUM,
    "SOC": VBIEntity_CLASS_MEASURE,
    "TTG": VBIEntity_CLASS_MEASURE,
    "VEBUS_VEBUS_ERROR": VBIEntity_CLASS_ENUM,
    "WARNING_REASON": VBIEntity_CLASS_ENUM,
}


PLATFORM_ENTITY_SCHEMA = "schema"
PLATFORM_ENTITY_INIT = "init"


# root schema to group (platform) VBIEntities linked to a Victron BLE device
def platform_schema(
    platform_entities: dict[str, cv.Schema],
):
    return cv.Schema(
        {
            cv.Required(CONF_VICTRON_BLE_IR_ID): cv.use_id(Manager),
        }
    ).extend({cv.Optional(type): schema for type, schema in platform_entities.items()})


async def platform_to_code(
    config: dict,
    platform_entities: dict[str, cv.Schema],
    init_func,
):
    manager = await cg.get_variable(config[CONF_VICTRON_BLE_IR_ID])

    for entity_key, entity_config in config.items():
        if entity_key in platform_entities:
            await init_func(entity_config, manager, VBIEntity_TYPE.enum(entity_key))


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
    return Array(*[int(part, 16) for part in parts])


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


def validate_auto_create_entities(value):
    value = cv.string_strict(value)
    if value not in VBIRecord_TYPES:
        raise cv.Invalid(f"Must be one of {VBIRecord_TYPES}")
    return value


CONFIG_SCHEMA = cv.All(
    cv.only_on_esp32,
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Manager),
            cv.Required(ec.CONF_MAC_ADDRESS): bind_mac_address_or_shortened,
            cv.Required(ec.CONF_BINDKEY): bind_key_array,
            cv.Optional(CONF_AUTO_CREATE_ENTITIES): validate_auto_create_entities,
            cv.Optional(
                CONF_LINK_CONNECTED_TIMEOUT, default="30s"
            ): cv.positive_time_period_seconds,
            cv.Optional(ec.CONF_ON_MESSAGE): automation.validate_automation(
                {
                    cv.GenerateID(ec.CONF_TRIGGER_ID): cv.declare_id(MessageTrigger),
                }
            ),
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[ec.CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)

    cg.add(var.set_address(config[ec.CONF_MAC_ADDRESS].as_hex))
    cg.add(var.set_bindkey(config[ec.CONF_BINDKEY].as_array))
    if CONF_AUTO_CREATE_ENTITIES in config:
        cg.add(
            var.set_auto_create_entities(
                VBIRecord_TYPE.enum(config[CONF_AUTO_CREATE_ENTITIES])
            )
        )
        if cv.Version(2025, 8, 0) <= ESPHOME_VERSION:
            # Add some reservation for core internal entity vectors if
            # we need to dynamically setup entities
            # BEWARE: we're skipping core api entry here
            # CORE.register_platform_component(self.snake_name, None)
            CORE.platform_counts["sensor"] += 20
            CORE.platform_counts["text_sensor"] += 10

    cg.add(var.set_link_connected_timeout(config[CONF_LINK_CONNECTED_TIMEOUT]))

    for conf in config.get(ec.CONF_ON_MESSAGE, []):
        trigger = cg.new_Pvariable(conf[ec.CONF_TRIGGER_ID], var)
        await automation.build_automation(
            trigger, [(VBIRecordConstPtr, "message")], conf
        )
