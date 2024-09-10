from esphome.components import sensor

from .. import (
    VBIEntity,
    VBIEntity_TYPES,
    m3_victron_ble_ir,
    platform_schema,
    platform_to_code,
)

VBISensor = m3_victron_ble_ir.class_("VBISensor", VBIEntity, sensor.Sensor)

_vbisensor_schema = sensor.sensor_schema(VBISensor)

PLATFORM_VBI_ENTITIES = {_type: _vbisensor_schema for _type in VBIEntity_TYPES}

CONFIG_SCHEMA = platform_schema(PLATFORM_VBI_ENTITIES)


async def to_code(config: dict):
    await platform_to_code(
        config,
        PLATFORM_VBI_ENTITIES,
        init_func=sensor.new_sensor,
    )
