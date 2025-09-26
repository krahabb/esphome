from esphome import config_validation as cv, const as ec
from esphome.components import number

from .. import m3_easun6048


async def _new_number(config: dict, *args):
    scale = config[m3_easun6048.CONF_SCALE]
    return await number.new_number(
        config,
        *args,
        min_value=config.get(ec.CONF_MIN_VALUE, 90 * scale),
        max_value=config.get(ec.CONF_MAX_VALUE, 160 * scale),
        step=config.get(ec.CONF_STEP, scale),
    )


_PLATFORM = m3_easun6048.Platform(
    number,
    frame_entity_schema=m3_easun6048.NUMERIC_FRAME_ENTITY_SCHEMA.extend(
        {
            cv.Optional(m3_easun6048.CONF_SCALE, default=0.1): cv.float_,
            cv.Optional(
                ec.CONF_ENTITY_CATEGORY, default=ec.ENTITY_CATEGORY_CONFIG
            ): cv.entity_category,
            cv.Optional(ec.CONF_MIN_VALUE): cv.float_,
            cv.Optional(ec.CONF_MAX_VALUE): cv.float_,
            cv.Optional(ec.CONF_STEP): cv.float_,
        }
    ),
    new_entity_func=_new_number,
)
CONFIG_SCHEMA = _PLATFORM.CONFIG_SCHEMA

to_code = _PLATFORM.to_code
