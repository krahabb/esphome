#pragma once
#include "esphome/components/text_sensor/text_sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class TextSensor : public esphome::text_sensor::TextSensor, public VEDirectEntity {
 public:
  void dynamic_register() override;
};

class HFTextSensor : public TextSensor, public HFEntity {
 public:
  /// @brief Builds a text sensor from a static definition
  /// @param def
  // TextSensor(Manager *manager, const VEDirectEntityDef &def);
  /// @brief Builds a text sensor for a TEXT frame or HEX register lacking definition
  /// @param text_name
  HFTextSensor(Manager *manager, register_id_t id) : HFEntity(manager, id) {}

  void parse_hex_value(const HexFrame *hexframe) override;
};

class TFTextSensor : public TextSensor, public TFEntity {
 public:
  TFTextSensor(Manager *manager, const char *label);

  void parse_text_value(const char *text_value) override;
};

}  // namespace m3_vedirect
}  // namespace esphome
