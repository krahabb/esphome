#pragma once

#include "esphome/core/log.h"
#include "esphome/components/sensor/sensor.h"
#include "../manager.h"
#include "../entity.h"

namespace esphome {
namespace m3_victron_ble_ir {

class VBISensor : public VBIEntity, public sensor::Sensor {
 public:
  static const char *UNITS[];
  static const char *DEVICE_CLASSES[];
  static const float DIGITS_TO_SCALE[];

  VBISensor(Manager *const manager, TYPE type);

  void link_disconnected() override;

 protected:
  float scale_;
  int32_t signed_offset_;

  bool init_(const RECORD_DEF *record_def) override;

  template<typename T> static void parse_bitmask_enum_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_signed_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_unsigned_t_(VBIEntity *entity, const VBI_RECORD *record);
  static void parse_cell_voltage_(VBIEntity *entity, const VBI_RECORD *record);
  static void parse_temperature_celsius_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_temperature_kelvin_(VBIEntity *entity, const VBI_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome