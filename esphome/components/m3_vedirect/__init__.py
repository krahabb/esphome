from enum import StrEnum
from functools import partial
import typing

from esphome import automation
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME, CONF_PAYLOAD, CONF_TRIGGER_ID
import esphome.cpp_generator as cpp

CODEOWNERS = ["@krahabb"]
DEPENDENCIES = ["binary_sensor", "select", "sensor", "switch", "text_sensor", "uart"]
AUTO_LOAD = ["binary_sensor", "select", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

m3_ve_reg_ns = cg.global_ns.namespace("m3_ve_reg")
HEXFRAME_struct = m3_ve_reg_ns.struct("HEXFRAME")
HEXFRAME_DATATYPE_enum = HEXFRAME_struct.enum("DATA_TYPE")


class DATA_TYPE(StrEnum):
    STRING = "STRING"
    UN8 = "UN8"
    UN16 = "UN16"
    UN32 = "UN32"
    SN8 = "SN8"
    SN16 = "SN16"
    SN32 = "SN32"


REG_DEF_struct = m3_ve_reg_ns.struct("REG_DEF")
REG_DEF_CLASS_enum = REG_DEF_struct.enum("CLASS")


class CLASS(StrEnum):
    BITMASK = "BITMASK"
    BOOLEAN = "BOOLEAN"
    ENUM = "ENUM"
    NUMERIC = "NUMERIC"
    STRING = "STRING"


ENUM_DEF_struct = m3_ve_reg_ns.struct("ENUM_DEF")
ENUM_DEF_LOOKUP_DEF_struct = ENUM_DEF_struct.struct("LOOKUP_DEF")


m3_vedirect_ns = cg.esphome_ns.namespace("m3_vedirect")
Manager = m3_vedirect_ns.class_("Manager", uart.UARTDevice, cg.Component)
HexFrame = m3_vedirect_ns.class_("HexFrame")
HexFrame_const_ref = HexFrame.operator("const").operator("ref")

HexFrameTrigger = Manager.class_(
    "HexFrameTrigger", automation.Trigger.template(HexFrame_const_ref)
)

CONF_VEDIRECT_ID = "vedirect_id"
CONF_VEDIRECT_ENTITIES = "vedirect_entities"
CONF_TEXTFRAME = "textframe"
CONF_HEXFRAME = "hexframe"

CONF_ENTITIES = "entities"
CONF_REGISTER_IDS = "register_ids"


# common schema for entities:
def validate_register_id():
    return cv.hex_int_range(min=0, max=65535)


def validate_enum_lookup_def(value):
    if not isinstance(value, dict) or len(value) != 1:
        raise cv.Invalid(
            f"Expected a single item dict with {{value}}: {{label}}, got {value}"
        )
    for enum_value, enum_label in value.items():
        return int(enum_value), enum_label


# CONF_REGISTER_ID = "register_id"
# CONF_CLASS = "class"
CONF_TEXT_LABEL = "text_label"
# (HEX) Register schema
CONF_REG_DEF_ID = "reg_def_id"
CONF_ENUM_DEF_ID = "enum_def_id"
CONF_ADDRESS = "address"
CONF_DATA_TYPE = "data_type"
VEDIRECT_REGISTER_SCHEMA = {
    cv.GenerateID(CONF_REG_DEF_ID): cv.declare_id(REG_DEF_struct),
    cv.GenerateID(CONF_ENUM_DEF_ID): cv.declare_id(ENUM_DEF_struct),
    # binds to the corresponding HEX register
    cv.Optional(CONF_ADDRESS): validate_register_id(),
    # configures the format of the HEX register
    cv.Optional(CONF_DATA_TYPE): cv.enum(
        {_dt.name: HEXFRAME_DATATYPE_enum.enum(_dt.name) for _dt in DATA_TYPE}
    ),
}
CONF_TEXT_SCALE = "text_scale"
CONF_SCALE = "scale"
CONF_UNIT = "unit"
VEDIRECT_REGISTER_CLASS_SCHEMAS = {
    CLASS.BOOLEAN: cv.Schema({}),
    CLASS.BITMASK: cv.ensure_list(validate_enum_lookup_def),
    CLASS.ENUM: cv.ensure_list(validate_enum_lookup_def),
    CLASS.NUMERIC: cv.Schema(
        {
            cv.Optional(CONF_SCALE): cv.float_,
            cv.Optional(CONF_TEXT_SCALE): cv.float_,
            cv.Optional(CONF_UNIT): cv.string,
        }
    ),
    CLASS.STRING: cv.Schema({}),
}
# TEXT record schema
VEDIRECT_TEXTRECORD_SCHEMA = {
    # binds to the corresponding TEXT frame field
    cv.Optional(CONF_TEXT_LABEL): cv.string,
}


CONF_TYPE = "type"
CONF_REGISTER = "register"


def vedirect_entity_schema(classes: typing.Iterable[CLASS], has_text):
    register_schema = dict(VEDIRECT_REGISTER_SCHEMA)
    for _cls in classes:
        register_schema |= {
            cv.Exclusive(_cls.name.lower(), "class"): VEDIRECT_REGISTER_CLASS_SCHEMAS[
                _cls
            ]
        }
    if has_text:
        register_schema |= VEDIRECT_TEXTRECORD_SCHEMA
    return {
        cv.Exclusive(CONF_TYPE, "type"): cv.string,
        cv.Exclusive(CONF_REGISTER, "type"): cv.Schema(register_schema),
    }


# Basic schema for binary-like entities: binary_sensor, switch
CONF_MASK = "mask"
VEDIRECT_BINARY_ENTITY_BASE_SCHEMA = {
    cv.Optional(CONF_MASK): cv.uint32_t,
}

"""
# TODO: add extended validator for VEDIRECT_ENTITY_BASE_SCHEMA to check the
# various combinations of possible options
def _entity_base_validator(config):
    if CONF_NAME not in config and CONF_ID not in config:
        raise Invalid("At least one of 'id:' or 'name:' is required!")
    if CONF_NAME not in config:
        id = config[CONF_ID]
        if not id.is_manual:
            raise Invalid("At least one of 'id:' or 'name:' is required!")
        config[CONF_NAME] = id.id
        config[CONF_INTERNAL] = True
        return config
    if config[CONF_NAME] is None:
        config[CONF_NAME] = ""
    return config
#cv.Schema.add_extra(_entity_base_validator)
"""

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


def local_object_construct(id_: cpp.ID, *args):
    obj = cpp.MockObj(id_, ".")
    expression = cpp.RawStatement(f"{id_.type} {id_}({cpp.ExpressionList(*args)});")
    cg.add(expression)
    return obj


async def new_vedirect_entity(config, manager):
    var = cg.new_Pvariable(config[CONF_ID], manager)
    valid = False

    if CONF_REGISTER in config:
        valid = True
        register_config = config[CONF_REGISTER]
        if CONF_ADDRESS in register_config:
            reg_def = local_object_construct(
                register_config[CONF_REG_DEF_ID], register_config[CONF_ADDRESS]
            )

            for _cls in CLASS:
                _cls_key = _cls.name.lower()
                if _cls_key in register_config:
                    cg.add(
                        cpp.AssignmentExpression(
                            None, "", reg_def.cls, REG_DEF_CLASS_enum.enum(_cls.name)
                        )
                    )
                    match _cls:
                        case CLASS.BITMASK | CLASS.ENUM:
                            enum_def = local_object_construct(
                                register_config[CONF_ENUM_DEF_ID],
                                register_config[_cls_key],
                            )
                            cg.add(
                                cpp.AssignmentExpression(
                                    None,
                                    "",
                                    reg_def.enum_def,
                                    cpp.UnaryOpExpression("&", enum_def),
                                )
                            )
                        case CLASS.NUMERIC:
                            pass
                    break

            if CONF_DATA_TYPE in register_config:
                cg.add(
                    cpp.AssignmentExpression(
                        None, "", reg_def.data_type, register_config[CONF_DATA_TYPE]
                    )
                )

            cg.add(var.set_reg_def(manager, cpp.UnaryOpExpression("&", reg_def)))

        if CONF_TEXT_LABEL in register_config:
            cg.add(var.set_text_label(manager, register_config[CONF_TEXT_LABEL]))

    # configure binary-like entities
    if CONF_MASK in config:
        cg.add(var.set_mask(config[CONF_MASK]))

    if not valid:
        raise cv.Invalid(f"Either {CONF_TYPE} or {CONF_REGISTER} must be provided")
    return var


async def vedirect_platform_to_code(
    config: dict,
    platform_entities: dict[str, cv.Schema],
    new_vedirectentity_func,
    new_entity_func,
):
    manager = await cg.get_variable(config[CONF_VEDIRECT_ID])

    for entity_key, entity_config in config.items():
        if entity_key == CONF_VEDIRECT_ENTITIES:
            for _entity_config in entity_config:
                await new_vedirectentity_func(_entity_config, manager)
            continue
        if entity_key in platform_entities:
            var = await new_entity_func(entity_config)
            cg.add(getattr(manager, f"set_{entity_key}")(var))


# main component (Manager) schema
CONF_AUTO_CREATE_ENTITIES = "auto_create_entities"
CONF_PING_TIMEOUT = "ping_timeout"
CONF_DEFAULT_ENTITIES = "default_entities"
DEFAULT_ENTITIES_MAP = {
    "mppt": (),
    "bmv60": (),
    "bmv70": (),
    "bmv71": (),
    "inverter": (),
    "charger": (),
}
CONF_ON_FRAME_RECEIVED = "on_frame_received"
CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Manager),
            cv.Optional(CONF_NAME): cv.string_strict,
            cv.Optional(CONF_DEFAULT_ENTITIES): cv.one_of(*DEFAULT_ENTITIES_MAP),
            cv.Optional(CONF_TEXTFRAME): cv.Schema(
                {
                    cv.Optional(CONF_AUTO_CREATE_ENTITIES): cv.boolean,
                }
            ),
            cv.Optional(CONF_HEXFRAME): cv.Schema(
                {
                    cv.Optional(CONF_AUTO_CREATE_ENTITIES): cv.boolean,
                    cv.Optional(CONF_PING_TIMEOUT): cv.positive_time_period_seconds,
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
        if CONF_AUTO_CREATE_ENTITIES in config_textframe:
            cg.add(
                var.set_auto_create_text_entities(
                    config_textframe[CONF_AUTO_CREATE_ENTITIES]
                )
            )

    if config_hexframe := config.get(CONF_HEXFRAME):
        if CONF_AUTO_CREATE_ENTITIES in config_hexframe:
            cg.add(
                var.set_auto_create_hex_entities(
                    config_hexframe[CONF_AUTO_CREATE_ENTITIES]
                )
            )
        if CONF_PING_TIMEOUT in config_hexframe:
            cg.add(var.set_ping_timeout(config_hexframe[CONF_PING_TIMEOUT]))

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
CONF_REGISTER_ID = "register_id"
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
