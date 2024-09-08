#include "entity.h"
namespace esphome {
namespace m3_victron_ble_ir {

/*
  These arrays map the availability of a specific field among the
  different record_types and its layout. They need to be defined in
  ascending order of record_type so that the init_func can
  succesfully lookup availability.

  @param record_type_: the type of record in which this field appears
  @param bit_offset_: the bit offset of the beginning of the field (according to
  https://wiki.victronenergy.com/rend/ble/extra_manufacturer_data)
  @param bit_size_: the number of bits that make up the field
*/

// clang-format off
/*
#define VBI_DEFINE_RECORD_DEF(record_type_, bit_offset_, bit_size_) \
  {record_type_, (bit_offset_ - 32) / 8, (bit_offset_ - 32 + bit_size_ + 7) / 8, (bit_offset_ - 32) % 8, \
   (1ULL << bit_size_) - 1},
*/
#define VBI_DEFINE_RECORD_DEF(...) {__VA_ARGS__},
#define VBI_DEFINE_RECORD_DEF_BITMASK(record_type, ...) {record_type, false, __VA_ARGS__, VBIEntity::DIGITS::D_0},
#define VBI_DEFINE_RECORD_DEF_ENUM(record_type, ...) {record_type, false, __VA_ARGS__, VBIEntity::DIGITS::D_0},
#define VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(record_type, ...) {record_type, true, __VA_ARGS__},
#define VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(record_type, ...) {record_type, false, __VA_ARGS__},
#define VBI_DEFINE_RECORD_DEF_TERMINATOR {VICTRON_BLE_RECORD::HEADER::_COUNT, false, 0, 0, VBIEntity::DIGITS::D_0}

const VBIEntity::RECORD_DEF AC_IN_ACTIVE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 78, 2)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::VE_BUS, 78, 2) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF AC_IN_REAL_POWER_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 80, 16, VBIEntity::DIGITS::D_0)
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::VE_BUS, 80, 19, VBIEntity::DIGITS::D_0)
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF AC_OUT_CURRENT_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER, 103, 11, VBIEntity::DIGITS::D_1)
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF AC_OUT_VOLTAGE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER, 88, 15, VBIEntity::DIGITS::D_2)
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF AC_OUT_APPARENT_POWER_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER, 72, 16, VBIEntity::DIGITS::D_0)
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF AC_OUT_REAL_POWER_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 112, 16, VBIEntity::DIGITS::D_0)
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 96, 16, VBIEntity::DIGITS::D_0)
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::VE_BUS, 99, 19, VBIEntity::DIGITS::D_0)
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF ALARM_NOTIFICATION_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::VE_BUS, 118, 2) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF ALARM_REASON_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::BATTERY_MONITOR, 64, 16) 
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::INVERTER, 40, 16) 
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 56, 16) 
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::DC_ENERGY_METER, 64, 16) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF BALANCER_STATUS_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::SMART_LITHIUM, 148, 4) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF BAT_TEMPERATURE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::SMART_LITHIUM, 152, 7, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::AC_CHARGER, 120, 7, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::LYNX_SMART_BMS, 152, 7, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::VE_BUS, 120, 7, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF CAH_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::BATTERY_MONITOR, 120, 20, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::LYNX_SMART_BMS, 132, 20, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF CHR_ERROR_CODE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 40, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::DCDC_CONVERTER, 40, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 40, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::AC_CHARGER, 40, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 48, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 40, 8)
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF CHR_TODAY_YIELD_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 80, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 96, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 128, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DC_CHANNEL1_CURRENT_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 64, 16, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::BATTERY_MONITOR, 98, 22, VBIEntity::DIGITS::D_3) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 64, 16, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::AC_CHARGER, 61, 11, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::LYNX_SMART_BMS, 72, 16, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 48, 16, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::VE_BUS, 48, 16, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::DC_ENERGY_METER, 98, 22, VBIEntity::DIGITS::D_3) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DC_CHANNEL1_VOLTAGE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 48, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::BATTERY_MONITOR, 48, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER, 56, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::DCDC_CONVERTER, 64, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::SMART_LITHIUM, 136, 12, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 48, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::GX_DEVICE, 32, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::AC_CHARGER, 48, 13, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 64, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::LYNX_SMART_BMS, 56, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 64, 14, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::VE_BUS, 64, 14, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_MEASURE_SIGNED(VICTRON_BLE_RECORD::HEADER::DC_ENERGY_METER, 48, 16, VBIEntity::DIGITS::D_2) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DC_INPUT_POWER_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 96, 16, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 80, 16, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::GX_DEVICE, 48, 20, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 112, 16, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DC_OUTPUT_STATUS_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 40, 8) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DEVICE_OFF_REASON_2_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::DCDC_CONVERTER, 80, 32) 
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 120, 32) 
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::ORION_XS, 112, 32) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DEVICE_STATE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 32, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::INVERTER, 32, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::DCDC_CONVERTER, 32, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 32, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::AC_CHARGER, 32, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 32, 8)
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 32, 8) 
  VBI_DEFINE_RECORD_DEF_ENUM(VICTRON_BLE_RECORD::HEADER::VE_BUS, 32, 8) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF SOC_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::BATTERY_MONITOR, 140, 10, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::GX_DEVICE, 68, 7, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::LYNX_SMART_BMS, 122, 10, VBIEntity::DIGITS::D_1) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::VE_BUS, 127, 7, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF TTG_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::BATTERY_MONITOR, 32, 16, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_MEASURE_UNSIGNED(VICTRON_BLE_RECORD::HEADER::LYNX_SMART_BMS, 40, 16, VBIEntity::DIGITS::D_0) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF WARNING_REASON_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF_BITMASK(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 72, 16) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};

#define VBI_DEFINE_DEFS(type_, ...) {type_, type_##_RECORD_TYPES, __VA_ARGS__},

const VBIEntity::DEF VBIEntity::DEFS[] = {VBIENTITIES(VBI_DEFINE_DEFS)};

// clang-format on
void VBIEntity::parse_init_(VBIEntity *entity, const VICTRON_BLE_RECORD *record) {
  // This is called the first time the entity/manager receives a record.
  // Here we'll customize the parsing based off actual entity 'type'
  // and incoming record type (which is unknown at compile time) and
  // 'fix' the parsing function so that subsequent calls will go straigth.
  // This is working under the condition that the 'record_type' will never change
  // after boot/initial connection and should be sure enough since the config
  // binds this component to a very specific physical device type.

  for (const auto *record_def = entity->def->record_types; record_def->record_type < VICTRON_BLE_RECORD::HEADER::_COUNT;
       ++record_def) {
    if (record_def->record_type == record->header.record_type) {
      entity->record_def_ = record_def;
      entity->data_begin_ = (record_def->bit_offset - 32) / 8;
      entity->data_end_ = (record_def->bit_offset - 32 + record_def->bit_size + 7) / 8;
      entity->data_shift_ = record_def->bit_offset % 8;
      entity->data_mask_ = (1ULL << record_def->bit_size) - 1;
      entity->nan_value_ = entity->data_mask_;  // by default. There are some exceptions though
      entity->raw_value_ = entity->nan_value_;
      entity->init_();
      entity->parse(record);
      return;
    }
  }
  // unsupported entity
  entity->init_unsupported_();
  return;
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome
