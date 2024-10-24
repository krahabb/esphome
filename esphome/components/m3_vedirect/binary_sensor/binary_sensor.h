#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class BinarySensor : public esphome::binary_sensor::BinarySensor, public VEDirectEntity {
 public:
  BinarySensor(Manager *manager) : VEDirectEntity(manager) {}

  void parse_text_value(const char *text_value) override { publish_state(!strcasecmp(text_value, "ON")); }

  void dynamic_register() override;

 protected:
  void init_reg_def_(const REG_DEF *reg_def) override;

  static void parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe);
};

}  // namespace m3_vedirect
}  // namespace esphome
