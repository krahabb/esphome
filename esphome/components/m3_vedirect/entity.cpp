#include "entity.h"

namespace esphome {
namespace m3_vedirect {

const char *NumericEntity::UNIT_TO_DEVICE_CLASS[REG_DEF::UNIT::UNIT_COUNT] = {
    nullptr, "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};

}  // namespace m3_vedirect
}  // namespace esphome
