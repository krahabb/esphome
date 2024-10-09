#include "binary_sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void HFBinarySensor::dynamic_register() {
  char *object_id = new char[7];
  sprintf(object_id, "0x%04X", (int) this->id);
  char *name = new char[16];
  sprintf(name, "Register %s", object_id);
  // name and object_id should likely need to be freed
  this->manager->setup_entity_name_id(this, name, object_id);

  App.register_binary_sensor(this);
  if (api::global_api_server) {
    add_on_state_callback([this](bool state) { api::global_api_server->on_binary_sensor_update(this, state); });
  }
}

TFBinarySensor::TFBinarySensor(Manager *manager, const char *label, const DEF *def) : TFEntity(manager, label, def) {
  manager->setup_entity_name_id(this, def ? def->description : label, label);
}

void TFBinarySensor::dynamic_register() {
  App.register_binary_sensor(this);
  if (api::global_api_server) {
    add_on_state_callback([this](bool state) { api::global_api_server->on_binary_sensor_update(this, state); });
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
