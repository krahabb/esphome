#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class HFBinarySensor : public esphome::binary_sensor::BinarySensor, public HFEntity {
 public:
  void dynamic_register() override;

  void parse_hex_value(const HexFrame *hexframe) override {
    if (hexframe->data_size() == sizeof(uint8_t)) {
      publish_state(hexframe->data_u8());
    }
  };
};

class TFBinarySensor : public esphome::binary_sensor::BinarySensor, public TFEntity {
 public:
  TFBinarySensor(Manager *manager, const char *label, const DEF *def);
  TFBinarySensor(Manager *manager, const char *label) : TFBinarySensor(manager, label, TFEntity::get_def(label)) {}

  void dynamic_register() override;

  void parse_text_value(const char *text_value) override { publish_state(!strcasecmp(text_value, "ON")); }
};

}  // namespace m3_vedirect
}  // namespace esphome
