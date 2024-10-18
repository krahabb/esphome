#include "sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void Sensor::parse_text_value(const char *text_value) {
  char *endptr;
  float value = strtof(text_value, &endptr) * this->text_scale_;
  if (*endptr != 0)
    value = NAN;
  if (value != this->raw_state)
    publish_state(value);
}

void Sensor::parse_hex_value(const HexFrame *hexframe) {
  // TODO(maybe): manage inconsistencies (log error?)
  // and improve if any the parsing
  float state;
  switch (this->hex_data_type_) {
    case HexFrame::DataType::u8:
      if (hexframe->data_size() != 1)
        return;
      state = hexframe->data_u8() / this->hex_scale_;
      break;
    case HexFrame::DataType::u16:
      if (hexframe->data_size() != 2)
        return;
      state = hexframe->data_u16() / this->hex_scale_;
      break;
    case HexFrame::DataType::i16:
      if (hexframe->data_size() != 2)
        return;
      state = hexframe->data_i16() / this->hex_scale_;
      break;
    case HexFrame::DataType::u32:
      if (hexframe->data_size() != 4)
        return;
      state = hexframe->data_u32() / this->hex_scale_;
      break;
    default:  // try to infer from data_size
      switch (hexframe->data_size()) {
        case 1:
          state = hexframe->data_u16() / this->hex_scale_;
          break;
        case 2:
          // it might be signed though
          state = hexframe->data_u16() / this->hex_scale_;
          break;
        case 4:
          state = hexframe->data_u16() / this->hex_scale_;
          break;
        default:
          return;
      }
  }

  if (state != this->raw_state)
    publish_state(state);
};

void Sensor::dynamic_register() {
  App.register_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](float state) { api::global_api_server->on_sensor_update(this, state); });
}

void Sensor::init_text_def_(const TEXT_DEF *text_def) {
  this->set_unit_of_measurement(UNITS[text_def->unit]);
  this->set_accuracy_decimals(text_def->digits);
  this->set_device_class(DEVICE_CLASSES[text_def->unit]);
  this->set_state_class(STATE_CLASSES[text_def->unit]);
  this->set_text_scale(DIGITS_TO_SCALE[text_def->digits]);
}

}  // namespace m3_vedirect
}  // namespace esphome
