#include "text_sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void TextSensor::parse_text_value(const char *text_value) {
  if (strcmp(this->raw_state.c_str(), text_value))
    this->publish_state(std::string(text_value));
}

void TextSensor::dynamic_register() {
  App.register_text_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](std::string state) { api::global_api_server->on_text_sensor_update(this, state); });
}

void TextSensor::init_reg_def_(const REG_DEF *reg_def) {
  if (reg_def) {
    switch (reg_def->cls) {
      case REG_DEF::CLASS::ENUM:
        this->enum_lookup_ = reg_def->enum_lookup;
        this->parse_hex_func_ = parse_hex_enum_;
        return;
      default:
        break;
    }
  }
  // defaults if nothing better
  this->parse_hex_func_ = parse_hex_default_;
}
void TextSensor::parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value)) {
    TextSensor *text_sensor = static_cast<TextSensor *>(entity);
    if (text_sensor->raw_state != hex_value)
      text_sensor->publish_state(hex_value);
  }
}
void TextSensor::parse_hex_enum_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  TextSensor *text_sensor = static_cast<TextSensor *>(entity);
  ENUM_DEF::data_type enum_value = hexframe->data_u8();
  if (text_sensor->enum_value_ != enum_value) {
    text_sensor->enum_value_ = enum_value;
    auto enum_label = text_sensor->enum_lookup_(enum_value);
    if (enum_label)
      text_sensor->publish_state(std::string(enum_label));
    else
      text_sensor->publish_state(std::to_string(enum_value));
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
