#include "sensor.h"

namespace esphome {
namespace m3_vedirect {

void Sensor::dynamic_register() {
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

struct TFSensorDef {
  struct TFEntityDef base;
  const char *const unit_of_measurement;
  const int accuracy_decimals;
  const float scale;
};
static const TFSensorDef TFSENSORS_DEF[] = {
    {"AC_OUT_I", "AC output current", "A", 1, 10},
    {"AC_OUT_S", "AC output apparent power", "VA", 0, 1},
    {"AC_OUT_V", "AC output voltage", "V", 2, 100},
    {"H19", "Yield total", "kWh", 2, 100},
    {"H20", "Yield today", "kWh", 2, 100},
    {"H21", "Maximum power today", "W", 0, 1},
    {"H22", "Yield yesterday", "kWh", 2, 100},
    {"H23", "Maximum power yesterday", "W", 0, 1},
    {"I", "Battery current", "A", 3, 1000},
    {"IL", "Load current", "A", 3, 1000},
    {"PPV", "PV power", "W", 0, 1},
    {"V", "Battery voltage", "V", 3, 1000},
    {"VPV", "PV voltage", "V", 3, 1000},
};

TFSensor::TFSensor(Manager *manager, const char *label) : TFEntity(manager, label) {
  set_object_id(label);
  for (const TFSensorDef &_def : TFSENSORS_DEF) {
    int _strcmp = strcmp(label, _def.base.label);
    if (_strcmp == 0) {
      set_name(_def.base.description);
      set_unit_of_measurement(_def.unit_of_measurement);
      set_accuracy_decimals(_def.accuracy_decimals);
      set_scale(_def.scale);
      break;
    } else if (_strcmp < 0) {
      break;
    }
  }
}
void TFSensor::parse_text_value(const char *text_value) {
  char *endptr;
  state = strtof(text_value, &endptr) / this->scale_;
  if (state != this->raw_state)
    publish_state(state);
}

}  // namespace m3_vedirect
}  // namespace esphome
