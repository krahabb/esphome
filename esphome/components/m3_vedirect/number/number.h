#pragma once
#include "esphome/components/number/number.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Number final : public ConfigEntity, public NumericEntity, public Entity, public esphome::number::Number {
 public:
  Number(Manager *manager) : ConfigEntity(manager), Entity(parse_hex_default_, parse_text_empty_) {}

 protected:
  friend class Manager;
  void dynamic_register_() override;
  void link_disconnected_() override;

  void init_reg_def_() override;

  static void parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_temperature_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  template<typename T> static void parse_hex_t_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static const parse_hex_func_t DATA_TYPE_TO_PARSE_HEX_FUNC_[];

  // interface esphome::number::Number

  void control(float value) override;
};

}  // namespace m3_vedirect
}  // namespace esphome
