#pragma once

#include "esphome/core/log.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "../manager.h"
#include "../entity.h"
namespace esphome {
namespace m3_victron_ble_ir {

class VBITextSensor : public VBIEntity, public text_sensor::TextSensor {
 public:
  VBITextSensor(TYPE type) : VBIEntity(type) {}

 protected:
  u_int32_t raw_value_;

  virtual void init_();

  static void parse_enum_(VBIEntity *_this, const VICTRON_BLE_RECORD *record);
  static void parse_enum_8bit_aligned_(VBIEntity *_this, const VICTRON_BLE_RECORD *record);
  static void parse_bitmask_(VBIEntity *entity, const VICTRON_BLE_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome