#pragma once
#include "esphome/components/text_sensor/text_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class TextSensor : public esphome::text_sensor::TextSensor, public VEDirectEntity {
 public:
  TextSensor(Manager *manager) : VEDirectEntity(manager) {}

  void parse_text_value(const char *text_value) override;

  void dynamic_register() override;

 protected:
  ENUM_DEF::data_type enum_value_{0xFF};
  ENUM_DEF::lookup_func_t enum_lookup_;

  void init_reg_def_(const REG_DEF *reg_def) override;
  static void parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe);
  static void parse_hex_enum_(VEDirectEntity *entity, const RxHexFrame *hexframe);
};

}  // namespace m3_vedirect
}  // namespace esphome
