#include "binary_sensor.h"

namespace esphome {
namespace m3_vedirect {

void BinarySensor::dynamic_register() {
  App.register_binary_sensor(this);
  if (api::global_api_server) {
    add_on_state_callback([this](bool state) { api::global_api_server->on_binary_sensor_update(this, state); });
  }
}

struct TFBinarySensorDef {
  struct TFEntityDef base;
};
static const TFBinarySensorDef TFBINARYSENSORS_DEF[] = {
    {"Alarm", "Alarm"},
    {"LOAD", "Output state"},
};

TFBinarySensor::TFBinarySensor(Manager *manager, const char *label) : TFEntity(manager, label) {
  set_object_id(label);
  for (const TFBinarySensorDef &_def : TFBINARYSENSORS_DEF) {
    int _strcmp = strcmp(label, _def.base.label);
    if (_strcmp == 0) {
      set_name(_def.base.description);
      break;
    } else if (_strcmp < 0) {
      break;
    }
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
