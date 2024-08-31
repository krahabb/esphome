#pragma once

#include "protocol.h"

namespace esphome {
namespace m3_victron_ble_ir {

#define VBIENTITIES(DEFINE_) \
  DEFINE_(AC_IN_ACTIVE, CLASS::ENUM, ENUM_VE_REG_AC_IN_ACTIVE::lookup) \
  DEFINE_(ALARM_NOTIFICATION, CLASS::ENUM, ENUM_VE_REG_ALARM_NOTIFICATION::lookup) \
  DEFINE_(ALARM_REASON, CLASS::BITMASK, ENUM_VE_REG_ALARM_REASON::lookup) \
  DEFINE_(BALANCER_STATUS, CLASS::ENUM, ENUM_VE_REG_BALANCER_STATUS::lookup) \
  DEFINE_(CHR_ERROR_CODE, CLASS::ENUM, ENUM_VE_REG_CHR_ERROR_CODE::lookup) \
  DEFINE_(DEVICE_OFF_REASON_2, CLASS::BITMASK, ENUM_VE_REG_DEVICE_OFF_REASON_2::lookup) \
  DEFINE_(DEVICE_STATE, CLASS::ENUM, ENUM_VE_REG_DEVICE_STATE::lookup) \
  DEFINE_(WARNING_REASON, CLASS::BITMASK, ENUM_VE_REG_WARNING_REASON::lookup)

#define VBI_DECLARE_ENUM(name_, ...) name_,

#define VBI_DECLARE_RECORD_DEFS(name_, ...) static const RECORD_DEF name_##_RECORD_TYPES[];

/// @brief Abstract base class for all of the entities
class VBIEntity {
 public:
  typedef void (*parse_func_t)(VBIEntity *_this, const VICTRON_BLE_RECORD *record);

  enum CLASS : u_int8_t {
    UNKNOWN = 0,
    ENUM,
    BITMASK,
    SIGNED,
    UNSIGNED,
  };

  enum TYPE : u_int8_t { VBIENTITIES(VBI_DECLARE_ENUM) };

  /// @brief This descriptor carries the mapping between an entity
  /// @brief and its layout in a specific VICTRON_BLE_RECORD type.
  struct RECORD_DEF {
    const VICTRON_BLE_RECORD::HEADER::TYPE record_type;
    // generalized raw data access would be:
    // (*(u_int32_t*)(raw_data + data_offset_) >> data_shift_) & data_mask_
    // care should be taken to not access-overflow (maybe) and eventually casting back the result

    // byte offset of field begin inside record rawdata
    const u_int8_t data_begin;
    // byte offset of field end inside record rawdata (for fields spawning multiple bytes)
    const u_int8_t data_end;
    // logical right shift to apply to rawdata when the field is not byte aligned
    const u_int8_t data_shift;
    const u_int32_t data_mask;

    static u_int8_t get_u_int8_t_aligned(const RECORD_DEF *record_def, const VICTRON_BLE_RECORD *record) {
      return record->data.raw[record_def->data_begin];
    }

    static u_int32_t get_u_int32_t(const RECORD_DEF *record_def, const VICTRON_BLE_RECORD *record) {
      return (*(u_int32_t *) (record->data.raw + record_def->data_begin) >> record_def->data_shift) &
             record_def->data_mask;
    }
  };
  /// @brief This descriptor carries the common properties
  /// @brief of an entity 'TYPE' such as 'parse_func'
  /// @brief and the list (map) of supporting VICTRON_BLE_RECORD types
  /// @brief and their layout related to the entity
  struct DEF {
    const TYPE type : 5;
    const CLASS cls : 3;
    void *const params;  // custom definition(s) based off 'cls'
    const RECORD_DEF *const record_types;
  };

  // VBIENTITIES(VBI_DECLARE_RECORD_DEFS)
  static const DEF DEFS[];

  const DEF *const def;

  VBIEntity(TYPE type) : def(&DEFS[type]) {}

  void parse(const VICTRON_BLE_RECORD *record) { this->parse_func_(this, record); }

  const RECORD_DEF *get_record_def() const { return this->record_def_; }

 protected:
  virtual void init_() {}
  void init_unsupported_() { this->parse_func_ = VBIEntity::parse_empty_; }

  void set_parse_func_(parse_func_t parse_func) { this->parse_func_ = parse_func; }

  static void parse_empty_(VBIEntity *entity, const VICTRON_BLE_RECORD *record) {}

 private:
  parse_func_t parse_func_{parse_init_};

  const RECORD_DEF *record_def_{};

  static void parse_init_(VBIEntity *_this, const VICTRON_BLE_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome