#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Sensor final : public NumericEntity, public Entity, public esphome::sensor::Sensor {
 public:
  // configuration symbols for numeric sensors
  static const sensor::StateClass UNIT_TO_STATE_CLASS[REG_DEF::UNIT::UNIT_COUNT];
  static const uint8_t SCALE_TO_DIGITS[REG_DEF::SCALE::SCALE_COUNT];

  Sensor(Manager *Manager) : Entity(parse_hex_default_, parse_text_default_) {}
  // REMOVE void set_hex_scale(float scale) { this->hex_scale_ = scale; }
  // REMOVE void set_text_scale(float scale) { this->text_scale_ = scale; }

 protected:
  friend class Manager;

  float text_scale_{1.};

  void dynamic_register_() override;
  void link_disconnected_() override;

  void init_reg_def_() override;

  static void parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_temperature_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  template<typename T> static void parse_hex_t_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static const parse_hex_func_t DATA_TYPE_TO_PARSE_HEX_FUNC_[];

  static void parse_text_default_(HexRegister *hex_register, const char *text_value);
};

}  // namespace m3_vedirect
}  // namespace esphome
