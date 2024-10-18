#include "text_sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void TextSensor::parse_text_value(const char *text_value) {
  if (strcmp(this->raw_state.c_str(), text_value))
    publish_state(std::string(text_value));
}

void TextSensor::parse_hex_value(const HexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value) && (this->raw_state != hex_value))
    publish_state(hex_value);
}

void TextSensor::dynamic_register() {
  App.register_text_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](std::string state) { api::global_api_server->on_text_sensor_update(this, state); });
}

}  // namespace m3_vedirect
}  // namespace esphome
