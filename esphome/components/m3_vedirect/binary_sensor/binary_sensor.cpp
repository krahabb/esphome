#include "binary_sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void BinarySensor::dynamic_register_() {
  App.register_binary_sensor(this);
  if (api::global_api_server) {
    add_on_state_callback([this](bool state) { api::global_api_server->on_binary_sensor_update(this, state); });
  }
}

void BinarySensor::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::BITMASK:
      this->parse_hex_ = parse_hex_bitmask_;
      break;
    case REG_DEF::CLASS::ENUM:
      this->parse_hex_ = parse_hex_enum_;
      break;
    default:
      // defaults if nothing better
      this->parse_hex_ = parse_hex_default_;
      break;
  }
}

void BinarySensor::parse_hex_default_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hexregister)->publish_state(hexframe->data_u8());
}
void BinarySensor::parse_hex_bitmask_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  BinarySensor *binary_sensor = static_cast<BinarySensor *>(hexregister);
  binary_sensor->publish_state(HEXFRAME::GET_DATA_AS_INT[binary_sensor->reg_def_->data_type](hexframe->record()) &
                               binary_sensor->mask_);
}
void BinarySensor::parse_hex_enum_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  BinarySensor *binary_sensor = static_cast<BinarySensor *>(hexregister);
  binary_sensor->publish_state(hexframe->data_u8() == binary_sensor->mask_);
}

void BinarySensor::parse_text_(const char *text_value) { publish_state(!strcasecmp(text_value, "ON")); }

void BinarySensor::parse_bitmask_(BITMASK_DEF::bitmask_t bitmask, const REG_DEF *reg_def) {
  this->publish_state(bitmask & this->mask_);
}

}  // namespace m3_vedirect
}  // namespace esphome
