#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Sensor : public esphome::sensor::Sensor, public VEDirectEntity {
 public:
  Sensor(Manager *manager) : VEDirectEntity(manager) {}

  void set_text_scale(float scale) { this->text_scale_ = scale; }
  void set_hex_scale(float scale) { this->hex_scale_ = scale; }
  void set_hex_data_type(HexFrame::DataType hex_data_type) { this->hex_data_type_ = hex_data_type; }

  void parse_text_value(const char *text_value) override;
  void parse_hex_value(const HexFrame *hexframe) override;

  void dynamic_register() override;

 protected:
  float text_scale_{1.};

  void init_text_def_(const TEXT_DEF *text_def) override;

  float hex_scale_{1.};
  HexFrame::DataType hex_data_type_{HexFrame::DataType::unknown};
};

}  // namespace m3_vedirect
}  // namespace esphome
