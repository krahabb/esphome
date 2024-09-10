#pragma once

#include "esphome/core/log.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../entity.h"

namespace esphome {
namespace m3_victron_ble_ir {

class VBIBinarySensor : public VBIEntity, public binary_sensor::BinarySensor {
 public:
  VBIBinarySensor(TYPE type);

  void set_mask(u_int32_t mask) { this->mask_ = mask; }

  void init(const RECORD_DEF *record_def) override;
  void link_disconnected() override;

 protected:
  u_int32_t mask_{0xFFFFFFFF};

  template<typename T> static void parse_bitmask_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_enum_t_(VBIEntity *entity, const VBI_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome