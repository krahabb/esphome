#include "sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void HFSensor::dynamic_register() {
  char *object_id = new char[7];
  sprintf(object_id, "0x%04X", (int) this->id);
  char *name = new char[16];
  sprintf(name, "Register %s", object_id);
  // name and object_id should likely need to be freed
  this->manager->setup_entity_name_id(this, name, object_id);

  App.register_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](float state) { api::global_api_server->on_sensor_update(this, state); });
}
/*
Sensor::Sensor(Manager *manager, const VEDirectEntityDef &def)
    : esphome::sensor::Sensor(), VEDirectEntity(manager, def), scale_(def.scale) {
  VEDirectEntity::init_entity_(this, def);
  set_unit_of_measurement(def.unit_of_measurement);
  set_accuracy_decimals(def.accuracy_decimals);
}
void Sensor::set_text_value(const char *text_value) {
  char *endptr;
  state = strtof(text_value, &endptr) / this->scale_;
  if (state != this->raw_state)
    publish_state(state);
}
void Sensor::parse_hex_value(const HexFrame *hexframe) {
  // TODO(maybe): manage inconsistencies (log error?)
  // and improve if any the parsing
  float state;
  switch (this->hex_data_type_) {
    case HexFrame::DataType::u8:
      if (hexframe->data_size() != 1)
        return;
      state = hexframe->data_u8() / this->scale_;
      break;
    case HexFrame::DataType::u16:
      if (hexframe->data_size() != 2)
        return;
      state = hexframe->data_u16() / this->scale_;
      break;
    case HexFrame::DataType::i16:
      if (hexframe->data_size() != 2)
        return;
      state = hexframe->data_i16() / this->scale_;
      break;
    case HexFrame::DataType::u32:
      if (hexframe->data_size() != 4)
        return;
      state = hexframe->data_u32() / this->scale_;
      break;
    default:  // try to infer from data_size
      switch (hexframe->data_size()) {
        case 1:
          state = hexframe->data_u16() / this->scale_;
          break;
        case 2:
          //it might be signed though
          state = hexframe->data_u16() / this->scale_;
          break;
        case 4:
          state = hexframe->data_u16() / this->scale_;
          break;
        default:
          return;
      }
  }

  if (state != this->raw_state)
    publish_state(state);
};
*/

TFSensor::TFSensor(Manager *manager, const char *label, const DEF *def) : TFEntity(manager, label, def) {
  if (def) {
    this->set_unit_of_measurement(UNITS[def->unit]);
    this->set_accuracy_decimals(def->digits);
    this->set_device_class(DEVICE_CLASSES[def->unit]);
    this->set_state_class(STATE_CLASSES[def->unit]);
    this->set_scale(DIGITS_TO_SCALE[def->digits]);
  }
  manager->setup_entity_name_id(this, def ? def->description : label, label);
}

void TFSensor::dynamic_register() {
  App.register_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](float state) { api::global_api_server->on_sensor_update(this, state); });
}

void TFSensor::parse_text_value(const char *text_value) {
  char *endptr;
  float value = strtof(text_value, &endptr) * this->scale_;
  if (*endptr != 0)
    value = NAN;
  if (value != this->raw_state)
    publish_state(value);
}

}  // namespace m3_vedirect
}  // namespace esphome
