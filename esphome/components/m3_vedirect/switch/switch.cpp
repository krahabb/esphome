#include "switch.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

Entity *Switch::build_entity(Manager *manager, const char *name, const char *object_id) {
  auto entity = new Switch(manager);
  Entity::dynamic_init_entity_(entity, name, object_id, manager->get_vedirect_name(), manager->get_vedirect_id());
  App.register_switch(entity);
  if (api::global_api_server)
    entity->add_on_state_callback([entity](bool state) { api::global_api_server->on_switch_update(entity, state); });
  return entity;
}

void Switch::init_reg_def_() {
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

void Switch::parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)->publish_state(hex_frame->data_t<uint8_t>());
}

void Switch::parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)->parse_bitmask_(hex_frame->safe_data_u32());
}

void Switch::parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<Switch *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}

void Switch::parse_text_default_(HexRegister *hex_register, const char *text_value) {
  static_cast<Switch *>(hex_register)->publish_state(!strcasecmp(text_value, "ON"));
}

void Switch::parse_text_bitmask_(HexRegister *hex_register, const char *text_value) {
  char *endptr;
  BITMASK_DEF::bitmask_t bitmask_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<Switch *>(hex_register)->parse_bitmask_(bitmask_value);
  }
}

void Switch::parse_text_enum_(HexRegister *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<Switch *>(hex_register)->parse_enum_(enum_value);
  }
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
  uint32_t hexvalue;
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::BITMASK:
      hexvalue = state ? this->raw_value_ | this->mask_ : this->raw_value_ & ~this->mask_;
      this->manager->send_register_set(this->reg_def_->register_id, &hexvalue, this->reg_def_->data_type);
      return;
    case REG_DEF::CLASS::ENUM:
      // what's a reasonable negation of mask_ ?
      this->manager->send_register_set(this->reg_def_->register_id, (ENUM_DEF::enum_t)(state ? this->mask_ : 0));
      return;
    default:
      // consider BOOLEAN
      this->manager->send_register_set(this->reg_def_->register_id, (uint8_t) (state ? 1 : 0));
      return;
  }
}
}  // namespace m3_vedirect
}  // namespace esphome
