#include "entity.h"
#include "esphome/core/helpers.h"
#include "manager.h"

namespace esphome {
namespace m3_victron_ble_ir {

// clang-format off
// clang-format on

#define VBI_DEFINE_DEFS(type_, ...) {type_, #type_, __VA_ARGS__},
const VBIEntity::DEF VBIEntity::DEFS[] = {VBIENTITIES(VBI_DEFINE_DEFS)};

#define VBI_DEFINE_RECORD_DEF_BITMASK(bit_offset, bit_size) \
  {false, bit_offset, bit_size, VBIEntity::DIGITS::D_0, VBIEntity::TYPE::_COUNT, 0},
#define VBI_DEFINE_RECORD_DEF_ENUM(bit_offset, bit_size) \
  {false, bit_offset, bit_size, VBIEntity::DIGITS::D_0, VBIEntity::TYPE::_COUNT, 0},
#define VBI_DEFINE_RECORD_DEF_SELECTOR(bit_offset, bit_size) \
  {false, bit_offset, bit_size, VBIEntity::DIGITS::D_0, VBIEntity::TYPE::_COUNT, 0},
#define VBI_DEFINE_RECORD_DEF_SIGNED(bit_offset, bit_size, digits) \
  {true, bit_offset, bit_size, digits, VBIEntity::TYPE::_COUNT, 0},
#define VBI_DEFINE_RECORD_DEF_UNSIGNED(bit_offset, bit_size, digits) \
  {false, bit_offset, bit_size, digits, VBIEntity::TYPE::_COUNT, 0},
#define VBI_DEFINE_RECORD_DEF_SIGNED_SELECTOR(bit_offset, bit_size, digits, selector_type, selector_value) \
  {true, bit_offset, bit_size, digits, selector_type, (u_int8_t) selector_value},
#define VBI_DEFINE_RECORD_DEF_UNSIGNED_SELECTOR(bit_offset, bit_size, digits, selector_type, selector_value) \
  {false, bit_offset, bit_size, digits, selector_type, (u_int8_t) selector_value},

const VBIEntity::entity_def_map_t VBIEntity::RECORD_DEFS{
    {VBI_RECORD::HEADER::SOLAR_CHARGER,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::CHR_ERROR_CODE, VBI_DEFINE_RECORD_DEF_ENUM(40, 8)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(48, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_SIGNED(64, 16, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::CHR_TODAY_YIELD, VBI_DEFINE_RECORD_DEF_UNSIGNED(80, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_INPUT_POWER, VBI_DEFINE_RECORD_DEF_UNSIGNED(96, 16, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::DC_OUTPUT_CURRENT, VBI_DEFINE_RECORD_DEF_UNSIGNED(112, 9, VBIEntity::DIGITS::D_1)},
     }},
    {VBI_RECORD::HEADER::BATTERY_MONITOR,
     {
         {VBIEntity::TYPE::TTG, VBI_DEFINE_RECORD_DEF_UNSIGNED(32, 16, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(48, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::ALARM_REASON, VBI_DEFINE_RECORD_DEF_BITMASK(64, 16)},
         {VBIEntity::TYPE::DC_CHANNEL2_VOLTAGE,
          VBI_DEFINE_RECORD_DEF_SIGNED_SELECTOR(80, 16, VBIEntity::DIGITS::D_2, VBIEntity::TYPE::BMV_AUX_INPUT,
                                                ENUM_VE_REG_BMV_AUX_INPUT::DC_CHANNEL2_VOLTAGE)},
         {VBIEntity::TYPE::BATTERY_MID_POINT_VOLTAGE,
          VBI_DEFINE_RECORD_DEF_UNSIGNED_SELECTOR(80, 16, VBIEntity::DIGITS::D_2, VBIEntity::TYPE::BMV_AUX_INPUT,
                                                  ENUM_VE_REG_BMV_AUX_INPUT::BATTERY_MID_POINT_VOLTAGE)},
         {VBIEntity::TYPE::BAT_TEMPERATURE,
          VBI_DEFINE_RECORD_DEF_UNSIGNED_SELECTOR(80, 16, VBIEntity::DIGITS::D_2, VBIEntity::TYPE::BMV_AUX_INPUT,
                                                  ENUM_VE_REG_BMV_AUX_INPUT::BAT_TEMPERATURE)},
         {VBIEntity::TYPE::BMV_AUX_INPUT, VBI_DEFINE_RECORD_DEF_SELECTOR(96, 2)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_SIGNED(98, 22, VBIEntity::DIGITS::D_3)},
         {VBIEntity::TYPE::CAH, VBI_DEFINE_RECORD_DEF_UNSIGNED(120, 20, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::SOC, VBI_DEFINE_RECORD_DEF_UNSIGNED(140, 10, VBIEntity::DIGITS::D_1)},
     }},
    {VBI_RECORD::HEADER::INVERTER,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::ALARM_REASON, VBI_DEFINE_RECORD_DEF_BITMASK(40, 16)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(56, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::AC_OUT_APPARENT_POWER, VBI_DEFINE_RECORD_DEF_UNSIGNED(72, 16, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::AC_OUT_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(88, 15, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::AC_OUT_CURRENT, VBI_DEFINE_RECORD_DEF_UNSIGNED(103, 11, VBIEntity::DIGITS::D_1)},
     }},
    {VBI_RECORD::HEADER::DCDC_CONVERTER,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::CHR_ERROR_CODE, VBI_DEFINE_RECORD_DEF_ENUM(40, 8)},
         {VBIEntity::TYPE::DC_INPUT_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(48, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(64, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DEVICE_OFF_REASON_2, VBI_DEFINE_RECORD_DEF_BITMASK(80, 32)},
     }},
    {VBI_RECORD::HEADER::SMART_LITHIUM,
     {
         {VBIEntity::TYPE::BMS_FLAGS, VBI_DEFINE_RECORD_DEF_ENUM(32, 32)},
         {VBIEntity::TYPE::SMART_LITHIUM_ERROR_FLAGS, VBI_DEFINE_RECORD_DEF_BITMASK(64, 16)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_1, VBI_DEFINE_RECORD_DEF_UNSIGNED(80, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_2, VBI_DEFINE_RECORD_DEF_UNSIGNED(87, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_3, VBI_DEFINE_RECORD_DEF_UNSIGNED(94, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_4, VBI_DEFINE_RECORD_DEF_UNSIGNED(101, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_5, VBI_DEFINE_RECORD_DEF_UNSIGNED(108, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_6, VBI_DEFINE_RECORD_DEF_UNSIGNED(115, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_7, VBI_DEFINE_RECORD_DEF_UNSIGNED(122, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BATTERY_CELL_VOLTAGE_8, VBI_DEFINE_RECORD_DEF_UNSIGNED(129, 7, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(136, 12, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::BALANCER_STATUS, VBI_DEFINE_RECORD_DEF_ENUM(148, 4)},
         {VBIEntity::TYPE::BAT_TEMPERATURE, VBI_DEFINE_RECORD_DEF_UNSIGNED(152, 7, VBIEntity::DIGITS::D_0)},
     }},
    {VBI_RECORD::HEADER::INVERTER_RS,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::CHR_ERROR_CODE, VBI_DEFINE_RECORD_DEF_ENUM(40, 8)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(48, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_SIGNED(64, 16, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::DC_INPUT_POWER, VBI_DEFINE_RECORD_DEF_UNSIGNED(80, 16, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::CHR_TODAY_YIELD, VBI_DEFINE_RECORD_DEF_UNSIGNED(96, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::AC_OUT_REAL_POWER, VBI_DEFINE_RECORD_DEF_SIGNED(112, 16, VBIEntity::DIGITS::D_0)},
     }},
    {VBI_RECORD::HEADER::GX_DEVICE,
     {
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(32, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_INPUT_POWER, VBI_DEFINE_RECORD_DEF_UNSIGNED(48, 20, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::SOC, VBI_DEFINE_RECORD_DEF_UNSIGNED(68, 7, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::DC_CHANNEL1_POWER, VBI_DEFINE_RECORD_DEF_SIGNED(75, 21, VBIEntity::DIGITS::D_0)},
     }},
    {VBI_RECORD::HEADER::AC_CHARGER,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::CHR_ERROR_CODE, VBI_DEFINE_RECORD_DEF_ENUM(40, 8)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(48, 13, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_UNSIGNED(61, 11, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::DC_CHANNEL2_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(72, 13, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL2_CURRENT, VBI_DEFINE_RECORD_DEF_UNSIGNED(85, 11, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::DC_CHANNEL3_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(96, 13, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL3_CURRENT, VBI_DEFINE_RECORD_DEF_UNSIGNED(109, 11, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::BAT_TEMPERATURE, VBI_DEFINE_RECORD_DEF_UNSIGNED(120, 7, VBIEntity::DIGITS::D_0)},
         //{VBIEntity::TYPE::AC_ACTIVE_INPUT_L1_CURRENT,},
     }},
    {VBI_RECORD::HEADER::SMART_BATTERY_PROTECT,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::DC_OUTPUT_STATUS, VBI_DEFINE_RECORD_DEF_ENUM(40, 8)},
         {VBIEntity::TYPE::CHR_ERROR_CODE, VBI_DEFINE_RECORD_DEF_ENUM(48, 8)},
         {VBIEntity::TYPE::ALARM_REASON, VBI_DEFINE_RECORD_DEF_BITMASK(56, 16)},
         {VBIEntity::TYPE::WARNING_REASON, VBI_DEFINE_RECORD_DEF_BITMASK(72, 16)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(88, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_OUTPUT_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(104, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DEVICE_OFF_REASON_2, VBI_DEFINE_RECORD_DEF_BITMASK(120, 32)},
     }},
    {VBI_RECORD::HEADER::LYNX_SMART_BMS,
     {
         {VBIEntity::TYPE::BMS_ERROR, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::TTG, VBI_DEFINE_RECORD_DEF_UNSIGNED(40, 16, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(56, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_SIGNED(72, 16, VBIEntity::DIGITS::D_1)},
         //{VBIEntity::TYPE::BMS_IO,},
         //{VBIEntity::TYPE::BMS_WARNINGS_ALARMS,},
         {VBIEntity::TYPE::SOC, VBI_DEFINE_RECORD_DEF_UNSIGNED(122, 10, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::CAH, VBI_DEFINE_RECORD_DEF_UNSIGNED(132, 20, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::BAT_TEMPERATURE, VBI_DEFINE_RECORD_DEF_UNSIGNED(152, 7, VBIEntity::DIGITS::D_0)},
     }},
    {VBI_RECORD::HEADER::MULTI_RS,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::CHR_ERROR_CODE, VBI_DEFINE_RECORD_DEF_ENUM(40, 8)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_SIGNED(48, 16, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(64, 14, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::AC_IN_ACTIVE, VBI_DEFINE_RECORD_DEF_SELECTOR(78, 2)},
         {VBIEntity::TYPE::AC_IN_1_REAL_POWER,
          VBI_DEFINE_RECORD_DEF_SIGNED_SELECTOR(80, 16, VBIEntity::DIGITS::D_0, VBIEntity::TYPE::AC_IN_ACTIVE,
                                                ENUM_VE_REG_AC_IN_ACTIVE::AC_IN_1)},
         {VBIEntity::TYPE::AC_IN_2_REAL_POWER,
          VBI_DEFINE_RECORD_DEF_SIGNED_SELECTOR(80, 16, VBIEntity::DIGITS::D_0, VBIEntity::TYPE::AC_IN_ACTIVE,
                                                ENUM_VE_REG_AC_IN_ACTIVE::AC_IN_2)},
         {VBIEntity::TYPE::AC_OUT_REAL_POWER, VBI_DEFINE_RECORD_DEF_SIGNED(96, 16, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::DC_INPUT_POWER, VBI_DEFINE_RECORD_DEF_UNSIGNED(112, 16, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::CHR_TODAY_YIELD, VBI_DEFINE_RECORD_DEF_UNSIGNED(128, 16, VBIEntity::DIGITS::D_2)},

     }},
    {VBI_RECORD::HEADER::VE_BUS,
     {
         {VBIEntity::TYPE::DEVICE_STATE, VBI_DEFINE_RECORD_DEF_ENUM(32, 8)},
         {VBIEntity::TYPE::VEBUS_VEBUS_ERROR, VBI_DEFINE_RECORD_DEF_ENUM(40, 8)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_SIGNED(48, 16, VBIEntity::DIGITS::D_1)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_UNSIGNED(64, 14, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::AC_IN_ACTIVE, VBI_DEFINE_RECORD_DEF_SELECTOR(78, 2)},
         {VBIEntity::TYPE::AC_IN_1_REAL_POWER,
          VBI_DEFINE_RECORD_DEF_SIGNED_SELECTOR(80, 19, VBIEntity::DIGITS::D_0, VBIEntity::TYPE::AC_IN_ACTIVE,
                                                ENUM_VE_REG_AC_IN_ACTIVE::AC_IN_1)},
         {VBIEntity::TYPE::AC_IN_2_REAL_POWER,
          VBI_DEFINE_RECORD_DEF_SIGNED_SELECTOR(80, 19, VBIEntity::DIGITS::D_0, VBIEntity::TYPE::AC_IN_ACTIVE,
                                                ENUM_VE_REG_AC_IN_ACTIVE::AC_IN_2)},
         {VBIEntity::TYPE::AC_OUT_REAL_POWER, VBI_DEFINE_RECORD_DEF_SIGNED(99, 19, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::ALARM_NOTIFICATION, VBI_DEFINE_RECORD_DEF_ENUM(118, 2)},
         {VBIEntity::TYPE::BAT_TEMPERATURE, VBI_DEFINE_RECORD_DEF_UNSIGNED(120, 7, VBIEntity::DIGITS::D_0)},
         {VBIEntity::TYPE::SOC, VBI_DEFINE_RECORD_DEF_UNSIGNED(127, 7, VBIEntity::DIGITS::D_0)},

     }},
    {VBI_RECORD::HEADER::DC_ENERGY_METER,
     {
         {VBIEntity::TYPE::BMV_MONITOR_MODE, VBI_DEFINE_RECORD_DEF_ENUM(32, 16)},
         {VBIEntity::TYPE::DC_CHANNEL1_VOLTAGE, VBI_DEFINE_RECORD_DEF_SIGNED(48, 16, VBIEntity::DIGITS::D_2)},
         {VBIEntity::TYPE::ALARM_REASON, VBI_DEFINE_RECORD_DEF_BITMASK(64, 16)},
         {VBIEntity::TYPE::DC_CHANNEL2_VOLTAGE,
          VBI_DEFINE_RECORD_DEF_SIGNED_SELECTOR(80, 16, VBIEntity::DIGITS::D_2, VBIEntity::TYPE::BMV_AUX_INPUT,
                                                ENUM_VE_REG_BMV_AUX_INPUT::DC_CHANNEL2_VOLTAGE)},
         {VBIEntity::TYPE::BAT_TEMPERATURE,
          VBI_DEFINE_RECORD_DEF_UNSIGNED_SELECTOR(80, 16, VBIEntity::DIGITS::D_2, VBIEntity::TYPE::BMV_AUX_INPUT,
                                                  ENUM_VE_REG_BMV_AUX_INPUT::BAT_TEMPERATURE)},
         {VBIEntity::TYPE::BMV_AUX_INPUT, VBI_DEFINE_RECORD_DEF_SELECTOR(96, 2)},
         {VBIEntity::TYPE::DC_CHANNEL1_CURRENT, VBI_DEFINE_RECORD_DEF_SIGNED(98, 22, VBIEntity::DIGITS::D_3)},
     }},
};

VBIEntity::VBIEntity(Manager *const manager, TYPE type) : def(DEFS[type]) { manager->register_entity(this); }

bool VBIEntity::init(const record_def_map_t &record_defs) {
  const auto record_def = record_defs.find(this->def.type);
  if (record_def == record_defs.end())
    // current record_type does not contain this entity TYPE
    return false;

  return this->init_(&record_def->second);
}

bool VBIEntity::init_(const RECORD_DEF *record_def) {
  this->record_def_ = record_def;
  this->data_begin_ = (record_def->bit_offset - 32) / 8;
  this->data_end_ = (record_def->bit_offset - 32 + record_def->bit_size + 7) / 8;
  this->data_shift_ = record_def->bit_offset % 8;
  this->data_mask_ = (1ULL << record_def->bit_size) - 1;
  this->nan_value_ = this->data_mask_;  // by default. There are some exceptions though
  this->raw_value_ = this->nan_value_;
  return true;
}

int VBIEntity::OBJECT_ID_COUNTER_ = 0;
const char *VBIEntity::calculate_object_id_() {
  this->object_id_str_ = str_sanitize(str_snake_case(this->def.label));
  this->object_id_str_ += ++OBJECT_ID_COUNTER_;
  return this->object_id_str_.c_str();
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome
