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

void Sensor::dynamic_register() {
  App.register_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](float state) { api::global_api_server->on_sensor_update(this, state); });
}

void Sensor::init_text_def_(const TEXT_DEF *text_def) {
  this->set_unit_of_measurement(REG_DEF::UNITS[text_def->unit]);
  this->set_accuracy_decimals(text_def->digits);
  this->set_device_class(UNIT_TO_DEVICE_CLASS[text_def->unit]);
  this->set_state_class(UNIT_TO_STATE_CLASS[text_def->unit]);
  this->set_text_scale(REG_DEF::DIGITS_TO_SCALE[text_def->digits]);
}

void Sensor::init_reg_def_(const REG_DEF *reg_def) {
  if (reg_def) {
    switch (reg_def->cls) {
      case REG_DEF::CLASS::NUMERIC:
        this->numeric_to_float_ = reg_def->numeric_to_float;
        this->parse_hex_func_ = parse_hex_numeric_;
        return;
      default:
        break;
    }
  }
  // defaults if nothing better
  this->parse_hex_func_ = parse_hex_default_;
}
void Sensor::parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  float value;
  switch (hexframe->data_size()) {
    case 1:
      value = hexframe->data_u16();
      break;
    case 2:
      // it might be signed though
      value = hexframe->data_u16();
      break;
    case 4:
      value = hexframe->data_u16();
      break;
    default:
      return;
  }
  Sensor *sensor = static_cast<Sensor *>(entity);
  if (sensor->raw_state != value) {
    sensor->publish_state(value);
  }
}
void Sensor::parse_hex_numeric_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  Sensor *sensor = static_cast<Sensor *>(entity);
  float value = sensor->numeric_to_float_(hexframe->data_begin());
  if (sensor->raw_state != value) {
    sensor->publish_state(value);
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
