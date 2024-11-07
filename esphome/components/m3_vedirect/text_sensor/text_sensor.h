#pragma once
#include "esphome/components/text_sensor/text_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class TextSensor final : public Entity, esphome::text_sensor::TextSensor {
 public:
  TextSensor(Manager *Manager) : Entity(parse_hex_default_, parse_text_default_) {}

 protected:
  friend class Manager;
  BITMASK_DEF::bitmask_t raw_value_{BITMASK_DEF::VALUE_UNKNOWN};

  void dynamic_register_() override;
  void link_disconnected_() override;

  void init_reg_def_() override;
  static void parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame);

  static void parse_text_default_(HexRegister *hex_register, const char *text_value);
  static void parse_text_bitmask_(HexRegister *hex_register, const char *text_value);
  static void parse_text_enum_(HexRegister *hex_register, const char *text_value);

  inline void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) override;
  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override;
  inline void parse_string_(const char *string_value) override;
};

}  // namespace m3_vedirect
}  // namespace esphome
