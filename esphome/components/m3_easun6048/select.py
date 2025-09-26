from esphome import codegen as cg, config_validation as cv, const as ec
from esphome.components import select

from .. import m3_easun6048


async def _new_select(config, *args):
    options = [option[1] for option in config[ec.CONF_OPTIONS]]
    entity = await select.new_select(config, *args, options=options)
    cg.add(
        getattr(entity, "set_options_map")(
            [option[0] for option in config[ec.CONF_OPTIONS]]
        )
    )
    # options.append(bytes([option[0] for option in config[ec.CONF_OPTIONS]]).decode("ascii"))
    return entity


def _validate_select_option(value):
    """Validate the configuration is a single item dict with {value}: {label}.
    representing a 'Select' option.
    """
    try:
        assert isinstance(value, str)
        index, label = value.split(":", 2)
        return cv.int_range(min=0, max=255)(index), cv.string_strict(label.strip())
    except Exception:
        raise cv.Invalid(
            f"Expected a string with format 'value:label', with value an integer 0-255 and label a string, got {value}"
        )


SELECT_OPTIONS_SCHEMA = cv.Schema([_validate_select_option])


def validate_select_options(value):
    """Validate this configuration option to be an ordered list of {value: 'label'}.

    If the config value is not a list, it is automatically converted to a
    single-item list.

    None and empty dictionaries are converted to empty lists.

    Code inspired by cv.ensure_list
    """

    cv.check_not_templatable(value)
    if value is None or (isinstance(value, dict) and not value):
        return []
    if not isinstance(value, list):
        return [_validate_select_option(value)]
    result = SELECT_OPTIONS_SCHEMA(value)
    option_value = -1
    for option in result:
        if option[0] <= option_value:
            raise cv.Invalid(f"Expected a list in ascending order, got {result}")
        option_value = option[0]

    return result


_PLATFORM = m3_easun6048.Platform(
    select,
    frame_entity_schema=m3_easun6048.FRAME_ENTITY_SCHEMA.extend(
        {
            cv.Optional(
                ec.CONF_ENTITY_CATEGORY, default=ec.ENTITY_CATEGORY_CONFIG
            ): cv.entity_category,
            cv.Required(ec.CONF_OPTIONS): validate_select_options,
        }
    ),
    new_entity_func=_new_select,
)

CONFIG_SCHEMA = _PLATFORM.CONFIG_SCHEMA

to_code = _PLATFORM.to_code
