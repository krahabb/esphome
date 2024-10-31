#include "entity.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "manager.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "m3_vedirect.entity";

const char *Entity::UNIT_TO_DEVICE_CLASS[] = {
    "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};
const sensor::StateClass Entity::UNIT_TO_STATE_CLASS[] = {
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_TOTAL,       sensor::StateClass::STATE_CLASS_TOTAL_INCREASING,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT,
};

void Entity::set_text_label(Manager *manager, const char *label) {
  auto text_def = TEXT_DEF::find_label(label);
  if (text_def) {
    this->init_text_def_(text_def);
    auto reg_def = REG_DEF::find_type(text_def->register_type);
    if (reg_def)
      this->set_reg_def(manager, reg_def);
  } else {
    this->init_text_def_(&TEXT_DEF_UNDEFINED);
  }
  manager->text_entities_.emplace(label, this);
}

void Entity::set_register_id(Manager *manager, register_id_t register_id) {
  auto reg_def = REG_DEF::find_register_id(register_id);
  this->set_reg_def(manager, reg_def ? reg_def : new REG_DEF(register_id));
}

}  // namespace m3_vedirect
}  // namespace esphome
