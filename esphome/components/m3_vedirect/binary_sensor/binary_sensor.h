#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class BinarySensor : public esphome::binary_sensor::BinarySensor, public VEDirectEntity {
 public:
  BinarySensor(Manager *manager) : VEDirectEntity(manager) {}

  void parse_text_value(const char *text_value) override { publish_state(!strcasecmp(text_value, "ON")); }
  void parse_hex_value(const HexFrame *hexframe) override {
    if (hexframe->data_size() == sizeof(uint8_t)) {
      publish_state(hexframe->data_u8());
    }
  };

  void dynamic_register() override;
};

}  // namespace m3_vedirect
}  // namespace esphome
