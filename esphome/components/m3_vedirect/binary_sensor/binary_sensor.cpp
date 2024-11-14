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
      this->parse_text_ = parse_text_bitmask_;
      break;
    case REG_DEF::CLASS::ENUM:
      this->parse_hex_ = parse_hex_enum_;
      this->parse_text_ = parse_text_enum_;
      break;
    default:
      break;
  }
}

void BinarySensor::parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  // By default considering the register as a BOOLEAN
  static_cast<BinarySensor *>(hex_register)->publish_state(hex_frame->data_t<ENUM_DEF::enum_t>());
}

void BinarySensor::parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hex_register)->parse_bitmask_(hex_frame->safe_data_u32());
}

void BinarySensor::parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}

void BinarySensor::parse_text_default_(HexRegister *hex_register, const char *text_value) {
  static_cast<BinarySensor *>(hex_register)->publish_state(!strcasecmp(text_value, "ON"));
}

void BinarySensor::parse_text_bitmask_(HexRegister *hex_register, const char *text_value) {
  char *endptr;
  BITMASK_DEF::bitmask_t bitmask_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<BinarySensor *>(hex_register)->parse_bitmask_(bitmask_value);
  }
}

void BinarySensor::parse_text_enum_(HexRegister *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<BinarySensor *>(hex_register)->parse_enum_(enum_value);
  }
}

void BinarySensor::parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) {
  this->publish_state(bitmask_value & this->mask_);
}

void BinarySensor::parse_enum_(ENUM_DEF::enum_t enum_value) { this->publish_state(enum_value == this->mask_); }

}  // namespace m3_vedirect
}  // namespace esphome
