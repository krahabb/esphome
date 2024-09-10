#pragma once

#include "esphome/core/log.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "../entity.h"
namespace esphome {
namespace m3_victron_ble_ir {

class VBITextSensor : public VBIEntity, public text_sensor::TextSensor {
 public:
  VBITextSensor(TYPE type);

  void init(const RECORD_DEF *record_def) override;
  void link_disconnected() override;

 protected:
  template<typename T> static void parse_bitmask_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_enum_t_(VBIEntity *entity, const VBI_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome