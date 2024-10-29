#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class BinarySensor : public Entity, public BitmaskParser, public esphome::binary_sensor::BinarySensor {
 public:
  BinarySensor(Manager *Manager) {}

  void set_mask(u_int32_t mask) { this->mask_ = mask; }

 protected:
  friend class Manager;

  u_int32_t mask_{0xFFFFFFFF};

  void dynamic_register_() override;
  void init_reg_def_() override;

  static void parse_hex_default_(HexRegister *hexregister, const RxHexFrame *hexframe);
  static void parse_hex_bitmask_(HexRegister *hexregister, const RxHexFrame *hexframe);
  static void parse_hex_enum_(HexRegister *hexregister, const RxHexFrame *hexframe);

  void parse_text_(const char *text_value) override;

  void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask, const REG_DEF *reg_def) override;
};

}  // namespace m3_vedirect
}  // namespace esphome
