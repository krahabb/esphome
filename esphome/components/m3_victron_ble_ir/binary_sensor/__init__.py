import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
import esphome.const as ec

from .. import (
    CONF_VICTRON_BLE_IR_ID,
    VBIEntity,
    VBIEntity_CLASS_ENUM,
    VBIEntity_TYPE,
    VBIEntity_TYPES,
    m3_victron_ble_ir,
    platform_schema,
)

VBIBinarySensor = m3_victron_ble_ir.class_(
    "VBIBinarySensor", VBIEntity, binary_sensor.BinarySensor
)

CONF_MASK = "mask"
_vbibinarysensor_schema = binary_sensor.binary_sensor_schema(VBIBinarySensor).extend(
    {cv.Required(CONF_MASK): cv.Any(cv.positive_int, cv.string)}
)


PLATFORM_VBI_ENTITIES = {
    _type: cv.ensure_list(_vbibinarysensor_schema)
    for _type, _class in VBIEntity_TYPES.items()
    if _class is VBIEntity_CLASS_ENUM
}
PLATFORM_MANAGER_ENTITIES = {
    cv.Optional("link_connected"): binary_sensor.binary_sensor_schema(
        binary_sensor.BinarySensorInitiallyOff,
        device_class=ec.DEVICE_CLASS_CONNECTIVITY,
        entity_category=ec.ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}
CONFIG_SCHEMA = platform_schema(PLATFORM_VBI_ENTITIES).extend(PLATFORM_MANAGER_ENTITIES)


async def to_code(config: dict):
    manager = await cg.get_variable(config[CONF_VICTRON_BLE_IR_ID])

    for entity_key, entity_config in config.items():
        if entity_key in PLATFORM_VBI_ENTITIES:
            entity_enum_class = m3_victron_ble_ir.enum(
                f"ENUM_VE_REG_{entity_key}", is_class=True
            )
            for entity_config_item in entity_config:
                entity = await binary_sensor.new_binary_sensor(
                    entity_config_item, manager, VBIEntity_TYPE.enum(entity_key)
                )
                mask = entity_config_item[CONF_MASK]
                if isinstance(mask, str):
                    mask = entity_enum_class.enum(mask)
                cg.add(entity.set_mask(mask))

        elif entity_key in PLATFORM_MANAGER_ENTITIES:
            entity = await binary_sensor.new_binary_sensor(entity_config)
            cg.add(getattr(manager, f"set_{entity_key}")(entity))
