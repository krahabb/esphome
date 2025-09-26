from collections.abc import Callable, Mapping
import contextlib
from types import ModuleType
from typing import Final

from esphome import codegen as cg, config_validation as cv
from esphome.components import uart
from esphome.cpp_helpers import CORE

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@krahabb"]
MULTI_CONF = True

CONF_EASUN6048_ID = "easun6048_id"
CONF_STATUS_FRAME = "status_frame"
CONF_CONFIG_FRAME = "config_frame"
CONF_TYPE = "type"
CONF_OFFSET = "offset"
CONF_SCALE = "scale"

m3_easun6048_ns = cg.esphome_ns.namespace("m3_easun6048")
Easun6048 = m3_easun6048_ns.class_("Easun6048", cg.PollingComponent, uart.UARTDevice)
Easun6048_SerialMode = Easun6048.enum("SerialMode", True)


def to_camel_case(name: str) -> str:
    """Convert a snake_case string to CamelCase."""
    return "".join(word.title() for word in name.split("_"))


PLATFORM_SCHEMA: Final = cv.Schema(
    {
        cv.Required(CONF_EASUN6048_ID): cv.use_id(Easun6048),
    }
)

# schema for entities linked to a component
FRAME_ENTITY_SCHEMA: Final = cv.Schema(
    {
        cv.Required(CONF_OFFSET): cv.positive_int,
    }
)

# additional schema for entities with numeric values
NUMERIC_FRAME_ENTITY_SCHEMA: Final = FRAME_ENTITY_SCHEMA.extend(
    {
        cv.Optional(CONF_SCALE, default=1.0): cv.float_,
        cv.Optional(CONF_TYPE, default="u8"): cv.enum(
            {
                "u8": cg.uint8,
                "u16": cg.uint16,
                "u32": cg.uint32,
                "i8": cg.global_ns.namespace("char"),
                "i16": cg.int16,
                "i32": cg.int32,
            }
        ),
    }
)


class Platform:
    def __init__(
        self,
        module: ModuleType,
        *,
        frame_entity_schema: cv.Schema | None = None,
        custom_entity_schema: dict[str, cv.Schema] = {},
        new_entity_func: Callable | None = None,
    ):
        self.name = module.__name__.split(".")[-1]
        self.module = module
        self.frame_entity_schema = frame_entity_schema
        self.custom_entity_schema = custom_entity_schema
        self.new_entity = new_entity_func or getattr(module, f"new_{self.name}")

    @property
    def CONFIG_SCHEMA(self):
        schema = {
            cv.Optional(_key): _schema
            for _key, _schema in self.custom_entity_schema.items()
        }
        if self.frame_entity_schema:
            entity_class_name = to_camel_case(self.name)
            entity_class_mock = m3_easun6048_ns.class_(
                entity_class_name, getattr(self.module, entity_class_name)
            )
            entity_base_schema = getattr(self.module, f"{self.name}_schema")
            entity_schema = entity_base_schema(entity_class_mock).extend(
                self.frame_entity_schema
            )
            schema[cv.Optional(CONF_STATUS_FRAME)] = cv.ensure_list(entity_schema)
            schema[cv.Optional(CONF_CONFIG_FRAME)] = cv.ensure_list(entity_schema)

        return PLATFORM_SCHEMA.extend(schema)

    async def to_code(self, config: Mapping):
        easun6048 = await cg.get_variable(config[CONF_EASUN6048_ID])
        if self.frame_entity_schema:
            for _key in (CONF_STATUS_FRAME, CONF_CONFIG_FRAME):
                for entity_config in config.get(_key, []):
                    if CONF_TYPE in entity_config:
                        entity = await self.new_entity(
                            entity_config,
                            cg.TemplateArguments(entity_config[CONF_TYPE]),
                        )
                    else:
                        entity = await self.new_entity(entity_config)
                    for _key2 in (CONF_OFFSET, CONF_SCALE):
                        with contextlib.suppress(KeyError):
                            cg.add(
                                getattr(entity, f"set_{_key2}")(entity_config[_key2])
                            )
                    cg.add(getattr(easun6048, f"bind_{_key}_entity")(entity))

        # handle custom entities not linked to a frame
        for _key in self.custom_entity_schema:
            if _key in config:
                entity = await self.new_entity(config[_key])
                cg.add(getattr(easun6048, f"set_{_key}_{self.name}")(entity))


CONF_SERIAL_MODE = "serial_mode"
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Easun6048),
            cv.Required(CONF_SERIAL_MODE): cv.All(
                cv.Upper,
                cv.enum(
                    {_i: Easun6048_SerialMode.enum(_i) for _i in ("MASTER", "SLAVE")}
                ),
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    antbms = cg.new_Pvariable(config[cv.CONF_ID])
    cg.add(antbms.set_serial_mode(config[CONF_SERIAL_MODE]))
    # In 'serial_mode == slave' the TX pin must be set to avoid bus contention
    # and we need to extract the TX pin from the UART config since the uart
    # interface doesnt expose that.
    uart_configs = CORE.config["uart"]
    for uart_config in uart_configs:
        if uart_config[cv.CONF_ID] == config[uart.CONF_UART_ID]:
            cg.add(antbms.set_tx_pin(uart_config[uart.CONF_TX_PIN][uart.CONF_NUMBER]))
            break
    await cg.register_component(antbms, config)
    await uart.register_uart_device(antbms, config)
