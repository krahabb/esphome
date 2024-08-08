#pragma once

#include "entity.h"
#include "hexframe.h"

namespace esphome {
namespace m3_vedirect {

class Manager;

class Factory {
 public:
  static TFEntity *build_entity(Manager *manager, const char *label);
  static HexRegister *build_register(Manager *manager, register_id_t register_id);
};

}  // namespace m3_vedirect
}  // namespace esphome
