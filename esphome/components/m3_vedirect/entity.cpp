#include "entity.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "manager.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "m3_vedirect.entity";

void Entity::set_register_type(Manager *manager, REG_DEF::TYPE register_type) {
  this->set_reg_def(manager, &REG_DEF::DEFS[register_type]);
  auto text_def = TEXT_DEF::find_type(register_type);
  if (text_def)
    manager->text_entities_.emplace(text_def->label, this);
}

void Entity::set_text_label(Manager *manager, const char *label) {
  auto text_def = TEXT_DEF::find_label(label);
  if (text_def) {
    if (this->reg_def_ == &REG_DEF::DEFS[REG_DEF::TYPE::UNDEFINED]) {
      // only set reg_def from our presets (if any) if the yaml generated code
      // didn't set a custom configuration
      auto reg_def = REG_DEF::find_type(text_def->register_type);
      if (reg_def)
        this->set_reg_def(manager, reg_def);
    }
  }
  manager->text_entities_.emplace(label, this);
}

}  // namespace m3_vedirect
}  // namespace esphome
