#include "switch.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void Switch::dynamic_register_() {
  App.register_switch(this);
  if (api::global_api_server) {
    this->add_on_state_callback([this](bool state) { api::global_api_server->on_switch_update(this, state); });
  }
}

void Switch::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::BITMASK:
      this->parse_hex_ = parse_hex_bitmask_;
      break;
    case REG_DEF::CLASS::ENUM:
      this->parse_hex_ = parse_hex_enum_;
      break;
    default:
      // defaults if nothing better
      this->parse_hex_ = parse_hex_enum_;
      break;
  }
}

void Switch::parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)
      ->parse_bitmask_(HEXFRAME::GET_DATA_AS_INT[hex_register->get_reg_def()->data_type](hex_frame->record()));
}

void Switch::parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)->parse_enum_(hex_frame->data_u8());
}

void Switch::parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) {
  if (this->raw_value_ != bitmask_value) {
    this->raw_value_ = bitmask_value;
    this->publish_state(bitmask_value & this->mask_);
  }
}
void Switch::parse_enum_(ENUM_DEF::enum_t enum_value) {
  if (this->raw_value_ != enum_value) {
    this->raw_value_ = enum_value;
    this->publish_state(enum_value == this->mask_);
  }
}

void Switch::write_state(bool state) {
  // This code should work for both ENUM-like and BITMASK-like registers
  // For the latter, actual bits are preserved so that we can toggle individual bits
  // inside the register. The 'mask' too might be used to control multiple bits at once.
  if (this->reg_def_) {
    uint32_t hexvalue;
    switch (this->reg_def_->cls) {
      case REG_DEF::CLASS::BITMASK:
        hexvalue = state ? this->raw_value_ | this->mask_ : this->raw_value_ & ~this->mask_;
        break;
      default:
        hexvalue = state ? this->mask_ : 0;  // what's a reasonable negation of mask_ ?
    }
    this->manager->send_register_set(this->reg_def_->register_id, &hexvalue, this->reg_def_->data_type);
  }
}
}  // namespace m3_vedirect
}  // namespace esphome
