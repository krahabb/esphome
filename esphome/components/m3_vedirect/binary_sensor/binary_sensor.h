#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class BinarySensor final : public Entity, public esphome::binary_sensor::BinarySensor {
 public:
  BinarySensor(Manager *Manager) : Entity(parse_hex_default_, parse_text_default_) {}

  void set_mask(uint32_t mask) { this->mask_ = mask; }

 protected:
  friend class Manager;

  uint32_t mask_{0xFFFFFFFF};

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
};

}  // namespace m3_vedirect
}  // namespace esphome
