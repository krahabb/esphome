#pragma once

#include "esphome/components/sensor/sensor.h"

#include "protocol.h"

#include <unordered_map>

namespace esphome {
namespace m3_victron_ble_ir {

class Manager;

#define VBIENTITIES(DEFINE_) \
  DEFINE_(AC_IN_ACTIVE, "Active AC in", SUBCLASS::SELECTOR, ENUM_VE_REG_AC_IN_ACTIVE::lookup) \
  DEFINE_(AC_IN_REAL_POWER, "AC input power", SUBCLASS::MEASURE, UNIT::W) \
  DEFINE_(AC_IN_1_REAL_POWER, "AC(1) input power", SUBCLASS::MEASURE, UNIT::W) \
  DEFINE_(AC_IN_2_REAL_POWER, "AC(2) input power", SUBCLASS::MEASURE, UNIT::W) \
  DEFINE_(AC_OUT_CURRENT, "AC output current", SUBCLASS::MEASURE, UNIT::A) \
  DEFINE_(AC_OUT_VOLTAGE, "AC output voltage", SUBCLASS::MEASURE, UNIT::V) \
  DEFINE_(AC_OUT_APPARENT_POWER, "AC output apparent power", SUBCLASS::MEASURE, UNIT::VA) \
  DEFINE_(AC_OUT_REAL_POWER, "AC output power", SUBCLASS::MEASURE, UNIT::W) \
  DEFINE_(ALARM_NOTIFICATION, "Alarm", SUBCLASS::ENUM, ENUM_VE_REG_ALARM_NOTIFICATION::lookup) \
  DEFINE_(ALARM_REASON, "Alarm reason", SUBCLASS::BITMASK, ENUM_VE_REG_ALARM_REASON::lookup) \
  DEFINE_(BALANCER_STATUS, "Balancer status", SUBCLASS::ENUM, ENUM_VE_REG_BALANCER_STATUS::lookup) \
  DEFINE_(BAT_TEMPERATURE, "Battery temperature", SUBCLASS::TEMPERATURE, UNIT::CELSIUS) \
  DEFINE_(BATTERY_CELL_VOLTAGE_1, "Cell(1) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_CELL_VOLTAGE_2, "Cell(2) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_CELL_VOLTAGE_3, "Cell(3) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_CELL_VOLTAGE_4, "Cell(4) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_CELL_VOLTAGE_5, "Cell(5) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_CELL_VOLTAGE_6, "Cell(6) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_CELL_VOLTAGE_7, "Cell(7) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_CELL_VOLTAGE_8, "Cell(8) voltage", SUBCLASS::CELLVOLTAGE, UNIT::V) \
  DEFINE_(BATTERY_MID_POINT_VOLTAGE, "Battery mid voltage", SUBCLASS::MEASURE, UNIT::V) \
  DEFINE_(BMS_ERROR, "BMS error", SUBCLASS::ENUM, ENUM_VE_REG_BMS_ERROR::lookup) \
  DEFINE_(BMS_FLAGS, "BMS flags", SUBCLASS::BITMASK, BITMASK_VE_REG_BMS_FLAGS::lookup) \
  DEFINE_(BMV_AUX_INPUT, "BMV aux input", SUBCLASS::SELECTOR, ENUM_VE_REG_BMV_AUX_INPUT::lookup) \
  DEFINE_(BMV_MONITOR_MODE, "BMV monitor mode", SUBCLASS::ENUM, ENUM_VE_REG_BMV_MONITOR_MODE::lookup) \
  DEFINE_(CAH, "Consumed AH", SUBCLASS::MEASURE, UNIT::Ah, sensor::StateClass::STATE_CLASS_TOTAL) \
  DEFINE_(CHR_ERROR_CODE, "Charger error", SUBCLASS::ENUM, ENUM_VE_REG_CHR_ERROR_CODE::lookup) \
  DEFINE_(CHR_TODAY_YIELD, "Yield today", SUBCLASS::MEASURE, UNIT::kWh, \
          sensor::StateClass::STATE_CLASS_TOTAL_INCREASING) \
  DEFINE_(DC_CHANNEL1_CURRENT, "Battery current", SUBCLASS::MEASURE, UNIT::A) \
  DEFINE_(DC_CHANNEL1_POWER, "Battery power", SUBCLASS::MEASURE, UNIT::W) \
  DEFINE_(DC_CHANNEL1_VOLTAGE, "Battery voltage", SUBCLASS::MEASURE, UNIT::V) \
  DEFINE_(DC_CHANNEL2_CURRENT, "Battery(2) current", SUBCLASS::MEASURE, UNIT::A) \
  DEFINE_(DC_CHANNEL2_VOLTAGE, "Battery(2) voltage", SUBCLASS::MEASURE, UNIT::V) \
  DEFINE_(DC_CHANNEL3_CURRENT, "Battery(3) current", SUBCLASS::MEASURE, UNIT::A) \
  DEFINE_(DC_CHANNEL3_VOLTAGE, "Battery(3) voltage", SUBCLASS::MEASURE, UNIT::V) \
  DEFINE_(DC_INPUT_VOLTAGE, "Input voltage", SUBCLASS::MEASURE, UNIT::V) \
  DEFINE_(DC_INPUT_POWER, "PV power", SUBCLASS::MEASURE, UNIT::W) \
  DEFINE_(DC_OUTPUT_CURRENT, "Output current", SUBCLASS::MEASURE, UNIT::A) \
  DEFINE_(DC_OUTPUT_STATUS, "Output state", SUBCLASS::ENUM, ENUM_VE_REG_DC_OUTPUT_STATUS::lookup) \
  DEFINE_(DC_OUTPUT_VOLTAGE, "output voltage", SUBCLASS::MEASURE, UNIT::V) \
  DEFINE_(DEVICE_OFF_REASON_2, "Off reason", SUBCLASS::BITMASK, ENUM_VE_REG_DEVICE_OFF_REASON_2::lookup) \
  DEFINE_(DEVICE_STATE, "Device state", SUBCLASS::ENUM, ENUM_VE_REG_DEVICE_STATE::lookup) \
  DEFINE_(SMART_LITHIUM_ERROR_FLAGS, "Error", SUBCLASS::BITMASK, ENUM_VE_REG_SMART_LITHIUM_ERROR_FLAGS::lookup) \
  DEFINE_(SOC, "State of charge", SUBCLASS::MEASURE, UNIT::SOC_PERCENTAGE) \
  DEFINE_(TTG, "Time to go", SUBCLASS::MEASURE, UNIT::minute) \
  DEFINE_(VEBUS_VEBUS_ERROR, "VEBUS error", SUBCLASS::ENUM, ENUM_VE_REG_VEBUS_VEBUS_ERROR::lookup) \
  DEFINE_(WARNING_REASON, "Warning reason", SUBCLASS::BITMASK, ENUM_VE_REG_WARNING_REASON::lookup)

#define VBI_DECLARE_ENUM(name_, ...) name_,

#define VBI_DECLARE_RECORD_DEFS(name_, ...) static const RECORD_DEF name_##_RECORD_TYPES[];

/// @brief Abstract base class for all of the entities
class VBIEntity {
 public:
  typedef void (*parse_func_t)(VBIEntity *_this, const VBI_RECORD *record);

  enum CLASS : u_int8_t {
    ENUMERATION,  // enumeration data or bitmask
    MEASUREMENT,  // numeric data (either signed or unsigned)
  };

  enum SUBCLASS : u_int8_t {
    BITMASK,   // represents a set of bit flags
    ENUM,      // represents a value among an enumeration
    SELECTOR,  // same as ENUM but used to select conditional parsing
    MEASURE,
    TEMPERATURE,
    CELLVOLTAGE,
  };

  // configuration symbols for numeric sensors
  enum UNIT : u_int8_t {
    A = 0,
    V = 1,
    VA = 2,
    W = 3,
    Ah = 4,
    kWh = 5,
    SOC_PERCENTAGE = 6,
    minute = 7,
    CELSIUS = 8,
  };

  enum DIGITS : u_int8_t {
    D_0 = 0,
    D_1 = 1,
    D_2 = 2,
    D_3 = 3,
  };

  enum TYPE : u_int8_t { VBIENTITIES(VBI_DECLARE_ENUM) _COUNT };

  /// @brief This descriptor carries the mapping between an entity
  /// @brief and its layout in a specific VBI_RECORD type.
  struct RECORD_DEF {
    const bool is_signed : 8;
    const u_int8_t bit_offset;
    const u_int8_t bit_size : 6;
    const DIGITS decimal_digits : 2;
    // Some entities share the same offset in the record
    // end their presence is indicated by the value of a corresponding enum
    const TYPE selector_type : 6;
    const u_int8_t selector_value : 2;
  } __attribute__((packed));
  /// @brief This descriptor carries the common properties
  /// @brief of an entity 'TYPE' such as 'parse_func'
  /// @brief and the list (map) of supporting VBI_RECORD types
  /// @brief and their layout related to the entity
  struct DEF {
    const TYPE type;
    const char *label;
    const char *default_name;
    const CLASS cls : 2;
    const SUBCLASS subcls : 6;
    union {
      // additional cfg for enum style entities
      EnumBase::lookup_func_t enum_lookup_func;
      // additional cfg for numeric entities
      struct {
        const UNIT unit;
        const sensor::StateClass state_class;
      };

    } __attribute__((packed));

    DEF(TYPE type, const char *label, const char *default_name, SUBCLASS subcls, EnumBase::lookup_func_t lookup_func)
        : type(type),
          label(label),
          default_name(default_name),
          cls(CLASS::ENUMERATION),
          subcls(subcls),
          enum_lookup_func(lookup_func) {}

    DEF(TYPE type, const char *label, const char *default_name, SUBCLASS subcls, UNIT unit,
        sensor::StateClass state_class = sensor::StateClass::STATE_CLASS_MEASUREMENT)
        : type(type),
          label(label),
          default_name(default_name),
          cls(CLASS::MEASUREMENT),
          subcls(subcls),
          unit(unit),
          state_class(state_class) {}
  };

  // Static list of defined entity TYPE(s) properties (struct DEF)
  static const DEF DEFS[TYPE::_COUNT];

  // Dictionary (unordered_map) of struct RECORD_DEF(s) indexed by record_type/TYPE
  typedef std::unordered_map<VBIEntity::TYPE, VBIEntity::RECORD_DEF> record_def_map_t;
  typedef std::unordered_map<VBI_RECORD::HEADER::TYPE, record_def_map_t> entity_def_map_t;
  static const entity_def_map_t RECORD_DEFS;
  /// @brief Returns a dictionary (indexed by TYPE) of RECORD_DEF(s) associated to record_type
  /// @param record_type
  /// @return nullptr if no dictionary defined for provided record_type
  static const record_def_map_t *get_record_defs(VBI_RECORD::HEADER::TYPE record_type) {
    const auto record_defs = VBIEntity::RECORD_DEFS.find(record_type);
    if (record_defs != VBIEntity::RECORD_DEFS.end()) {
      return &record_defs->second;
    }
    return nullptr;
  }

  const DEF &def;

  VBIEntity(Manager *const manager, TYPE type);

  const RECORD_DEF *get_record_def() const { return this->record_def_; }
  TYPE get_selector_type() const { return this->record_def_->selector_type; }

  u_int32_t get_raw_value() { return this->raw_value_; }

  virtual bool is_binary_sensor() { return false; }
  virtual bool is_text_sensor() { return false; }

  /// @brief to be called when record_type is known in order to
  /// @brief fix pointers to data record and setup proper parse_func
  /// @param record_defs: dictionary of RECORD_DEF structs (indexed by entity::TYPE) to lookup
  /// @return false if record_defs doesn't have a definition for this entity::TYPE
  bool init(const record_def_map_t &record_defs);

  void parse(const VBI_RECORD *record) { this->parse_func_(this, record); }

  // called by the Manager when BLE timeouts (we'll send 'unknown' to APIServer)
  virtual void link_disconnected() = 0;

 protected:
  friend class VBIBinarySensor;
  friend class VBISensor;
  friend class VBITextSensor;

  // byte offset of field begin inside record rawdata
  u_int8_t data_begin_;
  // byte offset of field end inside record rawdata (for fields spawning multiple bytes)
  u_int8_t data_end_;
  // logical right shift to apply to rawdata when the field is not byte aligned
  u_int8_t data_shift_;
  // mask to apply to the raw buffer read to extract the raw value
  u_int32_t data_mask_;
  // generic (any alignment/bit size) read into buffered record data
  inline u_int32_t read_record_(const VBI_RECORD *record) {
    return (*(u_int32_t *) (record->data.raw + this->data_begin_) >> this->data_shift_) & this->data_mask_;
  }
  // fast read 8 bit aligned
  inline u_int8_t read_record_u_int8_t_(const VBI_RECORD *record) {
    return *(u_int8_t *) (record->data.raw + this->data_begin_);
  }
  // fast read 16 bit aligned
  inline u_int16_t read_record_u_int16_t_(const VBI_RECORD *record) {
    return *(u_int16_t *) (record->data.raw + this->data_begin_);
  }
  // fast read 32 bit aligned
  inline u_int32_t read_record_u_int32_t_(const VBI_RECORD *record) {
    return *(u_int32_t *) (record->data.raw + this->data_begin_);
  }

  template<typename T> inline T read_record_t_(const VBI_RECORD *record);

  u_int32_t nan_value_;
  u_int32_t raw_value_;

  /// @brief Internally called by init() to allow descendants customizations
  /// @param record_def : the definition of the layout of this entity
  /// @return false if the entity is not supported/initialized
  virtual bool init_(const RECORD_DEF *record_def);

  void init_parse_func_(parse_func_t parse_func) { this->parse_func_ = parse_func; }

  // helper to dynamically setup entity object_id
  static int OBJECT_ID_COUNTER_;
  std::string object_id_str_;
  const char *calculate_object_id_();

 private:
  parse_func_t parse_func_{parse_empty_};

  const RECORD_DEF *record_def_{};

  static void parse_empty_(VBIEntity *entity, const VBI_RECORD *record) {}
};

/*
  read_record_t_ specialization:

  when accessing 8/16 bit aligned data we'll use a fast access
  while in general, for any unaligned or variable bit size we'll use the 32bit version
  which does the align/masking
*/
template<typename T> inline T VBIEntity::read_record_t_(const VBI_RECORD *record) {
  return *(T *) (record->data.raw + this->data_begin_);
}

template<> inline u_int32_t VBIEntity::read_record_t_<u_int32_t>(const VBI_RECORD *record) {
  return (*(u_int32_t *) (record->data.raw + this->data_begin_) >> this->data_shift_) & this->data_mask_;
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome