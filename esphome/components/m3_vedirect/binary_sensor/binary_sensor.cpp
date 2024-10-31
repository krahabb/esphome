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

void BinarySensor::parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hex_register)->publish_state(hex_frame->data_u8());
}

void BinarySensor::parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hex_register)
      ->parse_bitmask_(HEXFRAME::GET_DATA_AS_INT[hex_register->get_reg_def()->data_type](hex_frame->record()));
}

void BinarySensor::parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<BinarySensor *>(hex_register)->parse_enum_(hex_frame->data_u8());
}

void BinarySensor::init_text_def_(const TEXT_DEF *text_def) {
  switch (text_def->cls) {
    // When installing a specialized parse_text ensure the correct 'reg_def_' is in place
    case REG_DEF::CLASS::BITMASK:
      this->parse_text_ = parse_text_bitmask_;
      break;
    case REG_DEF::CLASS::ENUM:
      this->parse_text_ = parse_text_enum_;
      break;
    default:
      this->parse_text_ = parse_text_default_;
      break;
  }
}

void BinarySensor::parse_text_default_(HexRegister *hex_register, const char *text_value) {
  static_cast<BinarySensor *>(hex_register)->parse_string_(text_value);
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

void BinarySensor::parse_string_(const char *string_value) { this->publish_state(!strcasecmp(string_value, "ON")); }

}  // namespace m3_vedirect
}  // namespace esphome
