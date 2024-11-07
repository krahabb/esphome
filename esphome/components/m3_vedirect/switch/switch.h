#pragma once
#include "esphome/components/switch/switch.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Switch final : public ConfigEntity, public esphome::switch_::Switch {
 public:
  // TODO: setup a default text parser (even tho 'TEXT' entities are usually readonly
  // we might have a mapping to an R/W hex register - same as Select)
  Switch(Manager *manager) : ConfigEntity(manager, parse_hex_default_, parse_text_default_) {}

  void set_mask(uint32_t mask) { this->mask_ = mask; }

 protected:
  friend class Manager;

  BITMASK_DEF::bitmask_t raw_value_{BITMASK_DEF::VALUE_UNKNOWN};
  BITMASK_DEF::bitmask_t mask_{0x01};

  void dynamic_register_() override;
  void init_reg_def_() override;

  static void parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame);

  static void parse_text_default_(HexRegister *hex_register, const char *text_value);
  static void parse_text_bitmask_(HexRegister *hex_register, const char *text_value);
  static void parse_text_enum_(HexRegister *hex_register, const char *text_value);

  inline void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) override;
  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override;

  // interface esphome::switch_::Switch

  void write_state(bool state) override;
};

}  // namespace m3_vedirect
}  // namespace esphome
