#include "text_sensor.h"
#include "esphome/core/application.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include "../manager.h"

#include <cinttypes>

namespace esphome {
namespace m3_vedirect {

Entity *TextSensor::build_entity(Manager *manager, const char *name, const char *object_id) {
  auto entity = new TextSensor(manager);
  Entity::dynamic_init_entity_(entity, name, object_id, manager->get_vedirect_name(), manager->get_vedirect_id());
  App.register_text_sensor(entity);
#ifdef USE_API
  if (api::global_api_server)
    entity->add_on_state_callback(
        [entity](std::string state) { api::global_api_server->on_text_sensor_update(entity, state); });
#endif
  return entity;
}

void TextSensor::link_disconnected_() {
  this->raw_value_ = BITMASK_DEF::VALUE_UNKNOWN;
  this->publish_state("unknown");
}

void TextSensor::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::BITMASK:
      this->parse_hex_ = parse_hex_bitmask_;
      this->parse_text_ = parse_text_bitmask_;
      break;
    case REG_DEF::CLASS::ENUM:
      this->parse_hex_ = parse_hex_enum_;
      this->parse_text_ = parse_text_enum_;
      break;
    case REG_DEF::CLASS::STRING:
      this->parse_hex_ = parse_hex_string_;
      break;
    default:
      break;
  }
}

void TextSensor::parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  char hex_value[RxHexFrame::ALLOCATED_ENCODED_SIZE];
  if (hex_frame->data_to_hex(hex_value, RxHexFrame::ALLOCATED_ENCODED_SIZE)) {
    static_cast<TextSensor *>(hex_register)->parse_string_(hex_value);
  }
}

void TextSensor::parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<TextSensor *>(hex_register)->parse_bitmask_(hex_frame->safe_data_u32());
}

void TextSensor::parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<TextSensor *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}

void TextSensor::parse_hex_string_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_cast<TextSensor *>(hex_register)->parse_string_(hex_frame->data_str());
}

void TextSensor::parse_text_default_(HexRegister *hex_register, const char *text_value) {
  static_cast<TextSensor *>(hex_register)->parse_string_(text_value);
}

void TextSensor::parse_text_bitmask_(HexRegister *hex_register, const char *text_value) {
  // When parsing text records for BITMASK-like values, the TEXT protocol might sometime carry
  // decimal based values and sometimes hexadecimal base values. This should be automatically
  // handled by strtoumax
  char *endptr;
  BITMASK_DEF::bitmask_t bitmask_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<TextSensor *>(hex_register)->parse_bitmask_(bitmask_value);
  } else {
    static_cast<TextSensor *>(hex_register)->parse_string_(text_value);
  }
}

void TextSensor::parse_text_enum_(HexRegister *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0) {
    static_cast<TextSensor *>(hex_register)->parse_enum_(enum_value);
  } else {
    static_cast<TextSensor *>(hex_register)->parse_string_(text_value);
  }
}

void TextSensor::parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) {
  if (this->raw_value_ != bitmask_value) {
    this->raw_value_ = bitmask_value;
    std::string state;
    ENUM_DEF *enum_def = this->reg_def_->enum_def;
    for (uint8_t bit = 0; bitmask_value; ++bit) {
      if (bitmask_value & 0x01) {
        if (state.size())
          state += ",";
        state += enum_def->get_lookup(bit).lookup_def->label;
      }
      bitmask_value >>= 1;
    }
    this->publish_state(state);
  }
}

void TextSensor::parse_enum_(ENUM_DEF::enum_t enum_value) {
  if (this->raw_value_ != enum_value) {
    this->raw_value_ = enum_value;
    this->publish_state(std::string(this->reg_def_->enum_def->get_lookup(enum_value).lookup_def->label));
  }
}

void TextSensor::parse_string_(const char *string_value) {
  if (strcmp(this->raw_state.c_str(), string_value)) {
    this->raw_value_ = BITMASK_DEF::VALUE_UNKNOWN;
    this->publish_state(std::string(string_value));
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
