#include "text_sensor.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

void TextSensor::dynamic_register_() {
  App.register_text_sensor(this);
  if (api::global_api_server)
    add_on_state_callback([this](std::string state) { api::global_api_server->on_text_sensor_update(this, state); });
}

void TextSensor::link_disconnected_() {
  this->raw_value_ = -1;
  this->publish_state("unknown");
}

void TextSensor::init_reg_def_() {
  /*
    TextSensor could render BITMASK registers through 2 independent mechanics:
    - Registering the TextSensor as an HexRegister for the BITMASK register (this is not
    automatically implemented through our Manager::build_hex_register factory). 'init_reg_def_'
    will then be called for the case and we'll setup our parse_hex_bitmask_ parser.
    - Registering the TextSensor as a BitmaskParser for a BitmaskHexRegister (preferred way).
    This way the BitmaskHexRegister can dispatch the register state to multiple dependant entities
    like binary sensors or switches for example (using 'parse_bitmask_'). In this case, 'init_reg_def_'
    would not be called then.
    At any rate, when a TextSensor 'renders' the state of a BITMASK register it builds
    a string containing the labels for all of the active bits in the register.
  */
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

void TextSensor::parse_hex_default_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value)) {
    TextSensor *text_sensor = static_cast<TextSensor *>(hexregister);
    if (text_sensor->raw_state != hex_value)
      text_sensor->publish_state(hex_value);
  }
}

void TextSensor::parse_hex_bitmask_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<TextSensor *>(hexregister)
      ->parse_bitmask_(HEXFRAME::GET_DATA_AS_INT[hexregister->get_reg_def()->data_type](hexframe->record()),
                       hexregister->get_reg_def());
}

void TextSensor::parse_hex_enum_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  TextSensor *text_sensor = static_cast<TextSensor *>(hexregister);
  int32_t enum_value = hexframe->data_u8();
  if (text_sensor->raw_value_ != enum_value) {
    text_sensor->raw_value_ = enum_value;
    text_sensor->publish_state(std::string(text_sensor->reg_def_->enum_def->get_lookup(enum_value).lookup_def->label));
  }
}

void TextSensor::parse_text_(const char *text_value) {
  if (strcmp(this->raw_state.c_str(), text_value))
    this->publish_state(std::string(text_value));
}

void TextSensor::parse_bitmask_(BITMASK_DEF::bitmask_t bitmask, const REG_DEF *reg_def) {
  if (this->raw_value_ != bitmask) {
    this->raw_value_ = bitmask;
    std::string state;
    ENUM_DEF *enum_def = reg_def_->enum_def;
    uint8_t bitcount = HEXFRAME::DATA_TYPE_TO_SIZE[reg_def_->data_type] * 8;
    for (uint8_t bit = 0; bit < bitcount; ++bit) {
      if (bitmask & 0x01) {
        if (state.size())
          state += ",";
        state += enum_def->get_lookup(bit).lookup_def->label;
      }
      bitmask >>= 1;
    }
    this->publish_state(state);
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
