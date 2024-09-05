from esphome.components import binary_sensor

from .. import VBIEntity, m3_victron_ble_ir, platform_schema, platform_to_code

VBIBinarySensor = m3_victron_ble_ir.class_(
    "VBIBinarySensor", VBIEntity, binary_sensor.BinarySensor
)

_vbibinarysensor_schema = binary_sensor.binary_sensor_schema(VBIBinarySensor)

TYPES = []
PLATFORM_ENTITIES = {_type: _vbibinarysensor_schema for _type in TYPES}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config,
        PLATFORM_ENTITIES,
        init_func=binary_sensor.new_binary_sensor,
    )
