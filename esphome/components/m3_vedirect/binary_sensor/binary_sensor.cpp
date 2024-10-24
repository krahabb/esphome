#include "binary_sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void BinarySensor::dynamic_register() {
  App.register_binary_sensor(this);
  if (api::global_api_server) {
    add_on_state_callback([this](bool state) { api::global_api_server->on_binary_sensor_update(this, state); });
  }
}

void BinarySensor::init_reg_def_(const REG_DEF *reg_def) { this->parse_hex_func_ = parse_hex_default_; }

void BinarySensor::parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(entity)->publish_state(hexframe->data_u8());
}

}  // namespace m3_vedirect
}  // namespace esphome
