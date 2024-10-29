#pragma once
#include "esphome/components/text_sensor/text_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class TextSensor : public Entity, public BitmaskParser, esphome::text_sensor::TextSensor {
 public:
  TextSensor(Manager *Manager) {}

 protected:
  friend class Manager;
  int32_t raw_value_{-1};

  void dynamic_register_() override;
  void link_disconnected_() override;
  void init_reg_def_() override;

  static void parse_hex_default_(HexRegister *hexregister, const RxHexFrame *hexframe);
  static void parse_hex_bitmask_(HexRegister *hexregister, const RxHexFrame *hexframe);
  static void parse_hex_enum_(HexRegister *hexregister, const RxHexFrame *hexframe);

  void parse_text_(const char *text_value) override;

  void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask, const REG_DEF *reg_def) override;
};

}  // namespace m3_vedirect
}  // namespace esphome
