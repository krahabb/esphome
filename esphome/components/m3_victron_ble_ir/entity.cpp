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
#define VBI_DEFINE_RECORD_DEF(record_type_, bit_offset_, bit_size_) \
  {record_type_, (bit_offset_ - 32) / 8, (bit_offset_ - 32 + bit_size_ + 7) / 8, (bit_offset_ - 32) % 8, \
   (1UL << bit_size_) - 1},

#define VBI_DEFINE_RECORD_DEF_TERMINATOR {VICTRON_BLE_RECORD::HEADER::_TEMINATOR, 0, 0, 0, 0}

const VBIEntity::RECORD_DEF AC_IN_ACTIVE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 78, 2)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::VE_BUS, 78, 2) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF ALARM_NOTIFICATION_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::VE_BUS, 118, 2) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF ALARM_REASON_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::BATTERY_MONITOR, 64, 16) 
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::INVERTER, 40, 16) 
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 56, 16) 
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::DC_ENERGY_METER, 64, 16) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF BALANCER_STATUS_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SMART_LITHIUM, 148, 4) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF CHR_ERROR_CODE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 40, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::DCDC_CONVERTER, 40, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 40, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::AC_CHARGER, 40, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 48, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 40, 8)
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DEVICE_OFF_REASON_2_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::DCDC_CONVERTER, 80, 32) 
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 120, 32) 
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::ORION_XS, 112, 32) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF DEVICE_STATE_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SOLAR_CHARGER, 32, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::INVERTER, 32, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::DCDC_CONVERTER, 32, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::INVERTER_RS, 32, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::AC_CHARGER, 32, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 32, 8)
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::MULTI_RS, 32, 8) 
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::VE_BUS, 32, 8) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};
const VBIEntity::RECORD_DEF WARNING_REASON_RECORD_TYPES[] = {
  VBI_DEFINE_RECORD_DEF(VICTRON_BLE_RECORD::HEADER::SMART_BATTERY_PROTECT, 72, 16) 
  VBI_DEFINE_RECORD_DEF_TERMINATOR
};

#define VBI_DEFINE_DEFS(name_, class_, params_) {name_, class_, (void *) params_, name_##_RECORD_TYPES},

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

  for (const auto *record_def = entity->def->record_types;; ++record_def) {
    if (record_def->record_type < record->header.record_type) {
      continue;
    }
    if (record_def->record_type == record->header.record_type) {
      entity->record_def_ = record_def;
      entity->init_();
      entity->parse(record);
      return;
    }
    // unsupported entity
    entity->init_unsupported_();
    return;
  }
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome
