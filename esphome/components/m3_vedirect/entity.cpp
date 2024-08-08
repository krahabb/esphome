#include "entity.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "manager.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "m3_vedirect.entity";

HexRegister::HexRegister(Manager *manager, register_id_t id) : manager(manager), id(id) {
  manager->hex_registers_.emplace(id, this);
}

TFEntity::TFEntity(Manager *manager, const char *label) : label(label) { manager->text_entities_.emplace(label, this); }

}  // namespace m3_vedirect
}  // namespace esphome
