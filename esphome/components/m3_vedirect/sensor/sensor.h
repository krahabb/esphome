#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Sensor : public Entity, public esphome::sensor::Sensor {
 public:
  Sensor(Manager *Manager) {}
  void set_text_scale(float scale) { this->text_scale_ = scale; }

 protected:
  friend class Manager;
  float text_scale_{1.};

  void dynamic_register_() override;
  void link_disconnected_() override;

  REG_DEF::numeric_to_float_func_t numeric_to_float_;

  void init_reg_def_() override;
  static void parse_hex_default_(HexRegister *hexregister, const RxHexFrame *hexframe);
  static void parse_hex_numeric_(HexRegister *hexregister, const RxHexFrame *hexframe);

  void init_text_def_(const TEXT_DEF *text_def) override;
  void parse_text_(const char *text_value) override;
};

}  // namespace m3_vedirect
}  // namespace esphome
