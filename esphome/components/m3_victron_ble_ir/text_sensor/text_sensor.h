#pragma once

#include "esphome/core/log.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "../entity.h"
namespace esphome {
namespace m3_victron_ble_ir {

class VBITextSensor : public VBIEntity, public text_sensor::TextSensor {
 public:
  VBITextSensor(TYPE type);

 protected:
  virtual void init_();

  template<typename T> static void parse_bitmask_t_(VBIEntity *entity, const VICTRON_BLE_RECORD *record);
  template<typename T> static void parse_enum_t_(VBIEntity *entity, const VICTRON_BLE_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome