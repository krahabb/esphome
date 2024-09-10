#pragma once

#include "esphome/core/log.h"
#include "esphome/components/sensor/sensor.h"
#include "../manager.h"
#include "../entity.h"

namespace esphome {
namespace m3_victron_ble_ir {

enum class VICTRON_SENSOR_TYPE {
  UNSET,

  AUX_VOLTAGE,
  ERROR,
  INPUT_VOLTAGE,
  LOAD_CURRENT,
  MID_VOLTAGE,
  OUTPUT_VOLTAGE,
  TEMPERATURE,

  // SMART_LITHIUM
  BALANCER_STATUS,
  BMS_FLAGS,
  CELL1,
  CELL2,
  CELL3,
  CELL4,
  CELL5,
  CELL6,
  CELL7,
  CELL8,

  // LYNX_SMART_BMS
  IO_STATUS,
  WARNINGS_ALARMS,

  // VE_BUS
  ALARM,

  // DC_ENERGY_METER
  BMV_MONITOR_MODE,

  // ORION_XS
  OUTPUT_CURRENT,
  INPUT_CURRENT,
};

class VictronSensor : public Component, public sensor::Sensor, public Parented<Manager> {
 public:
  void setup() override;

  void set_type(VICTRON_SENSOR_TYPE val) { this->type_ = val; }

 protected:
  VICTRON_SENSOR_TYPE type_;
};

class VBISensor : public VBIEntity, public sensor::Sensor {
 public:
  static const char *UNITS[];
  static const char *DEVICE_CLASSES[];
  static const float DIGITS_TO_SCALE[];

  VBISensor(TYPE type);

  void init(const RECORD_DEF *record_def) override;

 protected:
  float scale_;
  int32_t signed_offset_;

  template<typename T> static void parse_bitmask_enum_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_signed_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_unsigned_t_(VBIEntity *entity, const VBI_RECORD *record);
  static void parse_temperature_(VBIEntity *entity, const VBI_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome