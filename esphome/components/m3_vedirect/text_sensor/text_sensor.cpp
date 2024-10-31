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
  this->raw_value_ = BITMASK_DEF::VALUE_UNKNOWN;
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
    static_cast<TextSensor *>(hexregister)->parse_string_(hex_value.c_str());
  }
}

void TextSensor::parse_hex_bitmask_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  // BITMASK registers have storage up to 4 bytes
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 4, "HexFrame storage might lead to access overflow");
  static_cast<TextSensor *>(hexregister)
      ->parse_bitmask_(HEXFRAME::GET_DATA_AS_INT[hexregister->get_reg_def()->data_type](hexframe->record()));
}

void TextSensor::parse_hex_enum_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<TextSensor *>(hexregister)->parse_enum_(hexframe->data_u8());
}

void TextSensor::init_text_def_(const TEXT_DEF *text_def) {
  switch (text_def->cls) {
    // When installing a specialized parse_text ensure the correct 'reg_def_' is in place
    case REG_DEF::CLASS::BITMASK:
      if ((this->reg_def_->cls == REG_DEF::CLASS::BITMASK) && (this->reg_def_->enum_def))
        this->parse_text_ = parse_text_bitmask_;
      else
        this->parse_text_ = parse_text_default_;
      break;
    case REG_DEF::CLASS::ENUM:
      if ((this->reg_def_->cls == REG_DEF::CLASS::ENUM) && (this->reg_def_->enum_def))
        this->parse_text_ = parse_text_enum_;
      else
        this->parse_text_ = parse_text_default_;
      break;
    default:
      this->parse_text_ = parse_text_default_;
      break;
  }
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
    uint8_t bitcount = HEXFRAME::DATA_TYPE_TO_SIZE[this->reg_def_->data_type] * 8;
    for (uint8_t bit = 0; bit < bitcount; ++bit) {
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
