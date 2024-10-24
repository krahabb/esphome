#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Sensor : public esphome::sensor::Sensor, public VEDirectEntity {
 public:
  Sensor(Manager *manager) : VEDirectEntity(manager) {}

  void set_text_scale(float scale) { this->text_scale_ = scale; }

  void parse_text_value(const char *text_value) override;

  void dynamic_register() override;

 protected:
  float text_scale_{1.};

  void init_text_def_(const TEXT_DEF *text_def) override;

  REG_DEF::numeric_to_float_func_t numeric_to_float_;

  void init_reg_def_(const REG_DEF *reg_def) override;
  static void parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe);
  static void parse_hex_numeric_(VEDirectEntity *entity, const RxHexFrame *hexframe);
};

}  // namespace m3_vedirect
}  // namespace esphome
