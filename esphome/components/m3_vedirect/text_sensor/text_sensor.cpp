#include "text_sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

//
//  TextSensor
//
void HFTextSensor::dynamic_register() {
  char *object_id = new char[7];
  sprintf(object_id, "0x%04X", (int) this->id);
  char *name = new char[16];
  sprintf(name, "Register %s", object_id);
  // name and object_id should likely need to be freed
  this->manager->setup_entity_name_id(this, name, object_id);
  App.register_text_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](std::string state) { api::global_api_server->on_text_sensor_update(this, state); });
}

void HFTextSensor::parse_hex_value(const HexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value) && (this->raw_state != hex_value))
    publish_state(hex_value);
}

TFTextSensor::TFTextSensor(Manager *manager, const char *label, const DEF *def) : TFEntity(manager, label, def) {
  manager->setup_entity_name_id(this, def ? def->description : label, label);
}

void TFTextSensor::dynamic_register() {
  App.register_text_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](std::string state) { api::global_api_server->on_text_sensor_update(this, state); });
}

void TFTextSensor::parse_text_value(const char *text_value) {
  if (strcmp(this->raw_state.c_str(), text_value))
    publish_state(std::string(text_value));
}

}  // namespace m3_vedirect
}  // namespace esphome
