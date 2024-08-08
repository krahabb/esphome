#include "text_sensor.h"

namespace esphome {
namespace m3_vedirect {

//
//  TextSensor
//
void TextSensor::dynamic_register() {
  App.register_text_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](std::string state) { api::global_api_server->on_text_sensor_update(this, state); });
}

void HFTextSensor::parse_hex_value(const HexFrame *hexframe) {
  auto hex_value = hexframe->data_hex();
  if (this->raw_state != hex_value)
    publish_state(hex_value);
}

struct TFTextSensorDef {
  struct TFEntityDef base;
};
static const TFTextSensorDef TFTEXTSENSORS_DEF[] = {
    {"AR", "Alarm reason"},
    {"CS", "State of operation"},
    {"ERR", "Error code"},
    {"FW", "Firmware version (FW)"},
    {"FWE", "Firmware version (FWE)"},
    {"MODE", "Device mode"},
    {"MPPT", "Tracker operation mode"},
    {"OR", "Off reason"},
    {"PID", "Product Id"},
    {"Relay", "Relay state"},
    {"SER#", "Serial number"},
    {"WARN", "Warning reason"},
};

TFTextSensor::TFTextSensor(Manager *manager, const char *label) : TFEntity(manager, label) {
  set_object_id(label);
  for (const TFTextSensorDef &_def : TFTEXTSENSORS_DEF) {
    int _strcmp = strcmp(label, _def.base.label);
    if (_strcmp == 0) {
      set_name(_def.base.description);
      break;
    } else if (_strcmp < 0) {
      break;
    }
  }
}
void TFTextSensor::parse_text_value(const char *text_value) {
  if (strcmp(this->raw_state.c_str(), text_value))
    publish_state(std::string(text_value));
}

}  // namespace m3_vedirect
}  // namespace esphome
