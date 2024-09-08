import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv

from .. import (
    CONF_VICTRON_BLE_IR_ID,
    TYPES,
    VBIEntity,
    VBIEntity_CLASS_BITMASK,
    VBIEntity_CLASS_ENUM,
    VBIEntity_TYPE,
    m3_victron_ble_ir,
    platform_schema,
)

VBIBinarySensor = m3_victron_ble_ir.class_(
    "VBIBinarySensor", VBIEntity, binary_sensor.BinarySensor
)

CONF_MASK = "mask"
_vbibinarysensor_schema = binary_sensor.binary_sensor_schema(VBIBinarySensor).extend(
    {cv.Optional(CONF_MASK, default=0xFFFFFFFF): cv.Any(cv.positive_int, cv.string)}
)


PLATFORM_ENTITIES = {
    _type: cv.ensure_list(_vbibinarysensor_schema)
    for _type, _class in TYPES.items()
    if _class in (VBIEntity_CLASS_BITMASK, VBIEntity_CLASS_ENUM)
}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    manager = await cg.get_variable(config[CONF_VICTRON_BLE_IR_ID])

    for entity_key, entity_config_list in config.items():
        if entity_key in PLATFORM_ENTITIES:
            entity_enum_class = m3_victron_ble_ir.enum(
                f"ENUM_VE_REG_{entity_key}", is_class=True
            )
            for entity_config in entity_config_list:
                entity = await binary_sensor.new_binary_sensor(
                    entity_config, VBIEntity_TYPE.enum(entity_key)
                )
                mask = entity_config[CONF_MASK]
                if isinstance(mask, str):
                    mask = entity_enum_class.enum(mask)
                cg.add(entity.set_mask(mask))
                cg.add(manager.register_entity(entity))
