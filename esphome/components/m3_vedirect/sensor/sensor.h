#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Sensor : public esphome::sensor::Sensor, public VEDirectEntity {
 public:
  void dynamic_register() override;
};

class HFSensor : public Sensor, public HFEntity {
 public:
  HFSensor(Manager *manager, register_id_t id) : HFEntity(manager, id) {}

  void set_hex_data_type(HexFrame::DataType hex_data_type) { this->hex_data_type_ = hex_data_type; }

  void parse_hex_value(const HexFrame *hexframe) override;

 protected:
  float scale_{1.};
  HexFrame::DataType hex_data_type_{HexFrame::DataType::unknown};
};

/*
template<typename T> class HexSensor : public Sensor {
 public:
  HexSensor(Manager *manager, const VEDirectEntityDef &def) : Sensor(manager, def) {}

  void parse_hex_value(const HexFrame *hexframe) override {
    if (hexframe->data_size() == sizeof(T)) {
      float state = (*(T *) hexframe->data_ptr()) / this->scale_;
      if (state != this->raw_state)
        publish_state(state);
    }
  };
};
*/

class TFSensor : public Sensor, public TFEntity {
 public:
  TFSensor(Manager *manager, const char *label);

  void set_scale(float scale) { scale_ = scale; }

  void parse_text_value(const char *text_value) override;

 protected:
  float scale_{1.};
};

}  // namespace m3_vedirect
}  // namespace esphome
