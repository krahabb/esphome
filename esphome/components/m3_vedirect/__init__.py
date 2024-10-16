from functools import partial

from esphome import automation
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME, CONF_PAYLOAD, CONF_TRIGGER_ID

CODEOWNERS = ["@krahabb"]
DEPENDENCIES = [
    "uart",
    "binary_sensor",
    "sensor",
    "text_sensor",
]
AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]
MULTI_CONF = True

m3_vedirect_ns = cg.esphome_ns.namespace("m3_vedirect")
Manager = m3_vedirect_ns.class_("Manager", uart.UARTDevice, cg.Component)
HexFrame = m3_vedirect_ns.class_("HexFrame")
HexFrame_const_ref = HexFrame.operator("const").operator("ref")

HexFrameTrigger = Manager.class_(
    "HexFrameTrigger", automation.Trigger.template(HexFrame_const_ref)
)

CONF_VEDIRECT_ID = "vedirect_id"
CONF_TEXTFRAME = "textframe"
CONF_HEXFRAME = "hexframe"

CONF_ENTITIES = "entities"
CONF_REGISTER_IDS = "register_ids"


# common schema for entities:
def validate_register_id():
    return cv.hex_int_range(min=0, max=65535)


HexDataType = HexFrame.enum("DataType")
DATA_TYPES = {
    "uint8": HexDataType.enum("u8"),
    "uint16": HexDataType.enum("u16"),
    "int16": HexDataType.enum("i16"),
    "uint32": HexDataType.enum("u32"),
}

CONF_REGISTER_ID = "register_id"
CONF_DATA_TYPE = "data_type"
CONF_LABEL = "label"
HEX_ENTITY_SCHEMA = cv.Schema(
    {
        # binds to the corresponding HEX register
        cv.Required(CONF_REGISTER_ID): validate_register_id(),
        # configures the format of the HEX register
        cv.Optional(CONF_DATA_TYPE): cv.enum(DATA_TYPES),
    }
)

TEXT_ENTITY_SCHEMA = cv.Schema(
    {
        # binds to the corresponding TEXT frame field
        cv.Required(CONF_LABEL): cv.string,
    }
)

# root schema to group (platform) entities linked to a Manager
VEDIRECT_PLATFORM_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_VEDIRECT_ID): cv.use_id(Manager),
    }
)


def vedirect_platform_schema(
    platform_entities: dict[str, cv.Schema],
):
    return VEDIRECT_PLATFORM_SCHEMA.extend(
        {cv.Optional(type): schema for type, schema in platform_entities.items()}
    )


async def vedirect_platform_to_code(
    config: dict,
    platform_entities: dict[str, cv.Schema],
    setup_entity_func,
    setup_hfentity_func,
    setup_tfentity_func,
):
    manager = await cg.get_variable(config[CONF_VEDIRECT_ID])

    for entity_key, entity_config in config.items():
        if entity_key == CONF_HEXFRAME:
            for _entity_config in entity_config:
                var = await setup_hfentity_func(
                    _entity_config, manager, _entity_config[CONF_REGISTER_ID]
                )
                if CONF_DATA_TYPE in _entity_config:
                    cg.add(var.set_hex_data_type(_entity_config[CONF_DATA_TYPE]))
            continue
        if entity_key == CONF_TEXTFRAME:
            for _entity_config in entity_config:
                var = await setup_tfentity_func(
                    _entity_config, manager, _entity_config[CONF_LABEL]
                )
            continue
        if entity_key in platform_entities:
            var = await setup_entity_func(entity_config)
            cg.add(getattr(manager, f"set_{entity_key}")(var))


# main component (Manager) schema
CONF_AUTO_CREATE_ENTITIES = "auto_create_entities"
CONF_DEFAULT_ENTITIES = "default_entities"
DEFAULT_ENTITIES_MAP = {
    CONF_TEXTFRAME: {
        "mppt": (),
        "bmv60": (),
        "bmv70": (),
        "bmv71": (),
        "inverter": (),
        "charger": (),
    },
    CONF_HEXFRAME: {
        "mppt": (),
        "bmv60": (),
        "bmv70": (),
        "bmv71": (),
        "inverter": (),
        "charger": (),
    },
}

CONF_ON_FRAME_RECEIVED = "on_frame_received"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Manager),
            cv.Optional(CONF_NAME): cv.string_strict,
            cv.Optional(CONF_TEXTFRAME): cv.Schema(
                {
                    cv.Optional(CONF_AUTO_CREATE_ENTITIES, default=True): cv.boolean,
                    cv.Optional(CONF_DEFAULT_ENTITIES): cv.one_of(
                        *DEFAULT_ENTITIES_MAP[CONF_TEXTFRAME]
                    ),
                }
            ),
            cv.Optional(CONF_HEXFRAME): cv.Schema(
                {
                    cv.Optional(CONF_AUTO_CREATE_ENTITIES, default=True): cv.boolean,
                    cv.Optional(CONF_DEFAULT_ENTITIES): cv.one_of(
                        *DEFAULT_ENTITIES_MAP[CONF_HEXFRAME]
                    ),
                    cv.Optional(CONF_ON_FRAME_RECEIVED): automation.validate_automation(
                        {
                            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                                HexFrameTrigger
                            ),
                        }
                    ),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config: dict):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_vedirect_id(str(var.base)))
    cg.add(var.set_vedirect_name(config.get(CONF_NAME, str(var.base))))
    if config_textframe := config.get(CONF_TEXTFRAME):
        cg.add(
            var.set_auto_create_text_entities(
                config_textframe[CONF_AUTO_CREATE_ENTITIES]
            )
        )

    if config_hexframe := config.get(CONF_HEXFRAME):
        cg.add(
            var.set_auto_create_hex_entities(config_hexframe[CONF_AUTO_CREATE_ENTITIES])
        )
        for conf in config_hexframe.get(CONF_ON_FRAME_RECEIVED, []):
            trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
            await automation.build_automation(
                trigger, [(HexFrame_const_ref, "hexframe")], conf
            )

    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)


# ACTIONS

_CTYPE_VALIDATOR_MAP = {
    cv.string: cg.std_string,
    cv.int_: cg.int_,
    validate_register_id: cg.uint16,
}


async def action_to_code(
    schema_def: dict[cv.Optional, object], config, action_id, template_args, args
):
    var = cg.new_Pvariable(action_id, template_args)
    for _schema_key, _ctype in schema_def.items():
        _key_name = _schema_key.schema
        if _key_name in config:
            template_ = await cg.templatable(
                config[_key_name], args, _CTYPE_VALIDATOR_MAP[_ctype]
            )
            cg.add(getattr(var, f"set_{_key_name}")(template_))
    return var


CONF_COMMAND = "command"
CONF_DATA = "data"
CONF_DATA_SIZE = "data_size"
MANAGER_ACTIONS = {
    "send_hexframe": {
        cv.Optional(CONF_VEDIRECT_ID, default=""): cv.string,
        cv.Required(CONF_PAYLOAD): cv.string,
    },
    "send_command": {
        cv.Optional(CONF_VEDIRECT_ID, default=""): cv.string,
        cv.Required(CONF_COMMAND): cv.int_,
        cv.Optional(CONF_REGISTER_ID): validate_register_id,
        cv.Optional(CONF_DATA): cv.int_,
        cv.Optional(CONF_DATA_SIZE): cv.int_,
    },
}

for _action_name, _schema_def in MANAGER_ACTIONS.items():
    _action = Manager.class_(f"Action_{_action_name}", automation.Action)
    _schema = cv.Schema(
        {
            _schema_key: cv.templatable(_ctype)
            for _schema_key, _ctype in _schema_def.items()
        }
    )
    automation.register_action(f"m3_vedirect.{_action_name}", _action, _schema)(
        partial(action_to_code, _schema_def)
    )
