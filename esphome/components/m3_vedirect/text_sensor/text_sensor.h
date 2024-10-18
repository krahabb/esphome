#pragma once
#include "esphome/components/text_sensor/text_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class TextSensor : public esphome::text_sensor::TextSensor, public VEDirectEntity {
 public:
  TextSensor(Manager *manager) : VEDirectEntity(manager) {}

  void parse_text_value(const char *text_value) override;
  void parse_hex_value(const HexFrame *hexframe) override;

  void dynamic_register() override;
};

}  // namespace m3_vedirect
}  // namespace esphome
