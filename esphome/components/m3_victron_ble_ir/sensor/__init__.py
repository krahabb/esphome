from esphome.components import sensor

from .. import TYPES, VBIEntity, m3_victron_ble_ir, platform_schema, platform_to_code

VBISensor = m3_victron_ble_ir.class_("VBISensor", VBIEntity, sensor.Sensor)

_vbisensor_schema = sensor.sensor_schema(VBISensor)

PLATFORM_ENTITIES = {_type: _vbisensor_schema for _type in TYPES}

CONFIG_SCHEMA = platform_schema(PLATFORM_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config,
        PLATFORM_ENTITIES,
        init_func=sensor.new_sensor,
    )
