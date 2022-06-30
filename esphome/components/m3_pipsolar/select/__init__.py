import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID
from .. import PIPSOLAR_COMPONENT_SCHEMA, CONF_PIPSOLAR_ID, pipsolar_ns

DEPENDENCIES = ["m3_pipsolar"]

PipsolarSelect = pipsolar_ns.class_("PipsolarSelect", select.Select)


CONF_POSSIBLE_VALUES = "possible_values"

# 3.11 PCVV<nn.n><cr>: Setting battery C.V. (constant voltage) charging voltage 48.0V ~ 58.4V for 48V unit
# battery_bulk_voltage;
# battery_recharge_voltage;     12V unit: 11V/11.3V/11.5V/11.8V/12V/12.3V/12.5V/12.8V
#                               24V unit: 22V/22.5V/23V/23.5V/24V/24.5V/25V/25.5V
#                               48V unit: 44V/45V/46V/47V/48V/49V/50V/51V
# battery_under_voltage;        40.0V ~ 48.0V for 48V unit
# battery_float_voltage;        48.0V ~ 58.4V for 48V unit
# battery_type;  00 for AGM, 01 for Flooded battery
# current_max_ac_charging_current;
# output_source_priority; 00 / 01 / 02
# charger_source_priority;  For HS: 00 for utility first, 01 for solar first, 02 for solar and utility, 03 for only solar charging
#                           For MS/MSX: 00 for utility first, 01 for solar first, 03 for only solar charging
# battery_redischarge_voltage;  12V unit: 00.0V12V/12.3V/12.5V/12.8V/13V/13.3V/13.5V/13.8V/14V/14.3V/14.5
#                               24V unit: 00.0V/24V/24.5V/25V/25.5V/26V/26.5V/27V/27.5V/28V/28.5V/29V
#                               48V unit: 00.0V48V/49V/50V/51V/52V/53V/54V/55V/56V/57V/58V


CONF_BATTERY_TYPE = "battery_type"
CONF_OUTPUT_SOURCE_PRIORITY = "output_source_priority"
CONF_CHARGER_SOURCE_PRIORITY = "charger_source_priority"
"""
CONF_BATTERY_RECHARGE_VOLTAGE = "battery_recharge_voltage"
CONF_BATTERY_UNDER_VOLTAGE = "battery_under_voltage"
CONF_BATTERY_FLOAT_VOLTAGE = "battery_float_voltage"
CONF_CURRENT_MAX_AC_CHARGING_CURRENT = "current_max_ac_charging_current"
CONF_CURRENT_MAX_CHARGING_CURRENT = "current_max_charging_current"
CONF_BATTERY_REDISCHARGE_VOLTAGE = "battery_redischarge_voltage"
CONF_BATTERY_BULK_VOLTAGE = "battery_bulk_voltage"
CONF_BATTERY_RECHARGE_VOLTAGE: (
    [44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0, 51.0],
    "PBCV%02.1f",
),
CONF_BATTERY_UNDER_VOLTAGE: (
    [40.0, 40.1, 42, 43, 44, 45, 46, 47, 48.0],
    "PSDV%02.1f",
),
CONF_BATTERY_FLOAT_VOLTAGE: ([48.0, 49.0, 50.0, 51.0], "PBFT%02.1f"),

CONF_CURRENT_MAX_AC_CHARGING_CURRENT: ([2, 10, 20], "MUCHGC0%02.0f"),
CONF_CURRENT_MAX_CHARGING_CURRENT: ([10, 20, 30, 40], "MCHGC0%02.0f"),
CONF_BATTERY_REDISCHARGE_VOLTAGE: (
    [0, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58],
    "PBDV%02.1f",
),
CONF_BATTERY_BULK_VOLTAGE: ([48.0, 49.0, 50.0, 51.0], "PCVV%02.1f"),
"""


TYPES = {
    CONF_BATTERY_TYPE: ("PBT%02.0f", [0, 1, 2], ["AGM", "Flooded", "User"]),
    CONF_OUTPUT_SOURCE_PRIORITY: (
        "POP%02.0f",
        [0, 1, 2],
        ["Utility", "Solar", "Battery"],
    ),
    CONF_CHARGER_SOURCE_PRIORITY: (
        "PCP%02.0f",
        [0, 1, 2, 3],
        ["Utility first", "Solar first", "Solar+Utility", "Solar only"],
    ),
}

CONFIG_SCHEMA = PIPSOLAR_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(type): select.SELECT_SCHEMA.extend(
            {
                cv.Required(CONF_ID): cv.declare_id(PipsolarSelect),
            }
        )
        for type in TYPES
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_PIPSOLAR_ID])

    for type, (command, values, traits) in TYPES.items():
        if type in config:
            conf = config[type]
            var = cg.new_Pvariable(conf[CONF_ID])
            await select.register_select(
                var, conf, options=values if traits is None else traits
            )
            cg.add(var.set_parent(paren))
            cg.add(var.set_set_command(command))
            cg.add(var.set_traits_values(values))
