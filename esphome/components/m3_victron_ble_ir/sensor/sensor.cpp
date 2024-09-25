#include "sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"

namespace esphome {
namespace m3_victron_ble_ir {

static const char *const TAG = "m3_victron_ble_ir.sensor";

const char *VBISensor::UNITS[] = {
    "A", "V", "VA", "W", "Ah", "kWh", "%", "min", "°C",
};
const char *VBISensor::DEVICE_CLASSES[] = {
    "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};
const float VBISensor::DIGITS_TO_SCALE[] = {1.f, .1f, .01f, .001f};

VBISensor::VBISensor(Manager *const manager, TYPE type) : VBIEntity(manager, type) {
  const auto def = this->def;
  this->set_name(def.default_name);
  this->set_object_id(this->calculate_object_id_());
  switch (def.cls) {
    case CLASS::MEASUREMENT:
      this->set_unit_of_measurement(UNITS[def.unit]);
      this->set_device_class(DEVICE_CLASSES[def.unit]);
      this->set_state_class(def.state_class);
      return;
    default:
      return;
  }
}

void VBISensor::link_disconnected() {
  if (this->raw_value_ != this->nan_value_) {
    this->raw_value_ = this->nan_value_;
    this->publish_state(NAN);
  }
}

bool VBISensor::init_(const RECORD_DEF *record_def) {
  this->VBIEntity::init_(record_def);
  switch (this->def.cls) {
    case CLASS::MEASUREMENT:
      this->set_accuracy_decimals(record_def->decimal_digits);
      this->scale_ = DIGITS_TO_SCALE[record_def->decimal_digits];
      switch (this->def.subcls) {
        case SUBCLASS::MEASURE:
          if (record_def->is_signed) {
            this->signed_offset_ = this->data_mask_ + 1;
            this->nan_value_ = this->data_mask_ >> 1;
            if (this->data_shift_ == 0) {
              if (this->data_mask_ == 0xFFFF) {
                this->init_parse_func_(parse_signed_t_<u_int16_t>);
                return true;
              }
              if (this->data_mask_ == 0xFF) {
                this->init_parse_func_(parse_signed_t_<u_int8_t>);
                return true;
              }
            }
            this->init_parse_func_(parse_signed_t_<u_int32_t>);
            return true;
          } else {  // unsigned
            if (this->data_shift_ == 0) {
              if (this->data_mask_ == 0xFFFF) {
                this->init_parse_func_(parse_unsigned_t_<u_int16_t>);
                return true;
              }
              if (this->data_mask_ == 0xFF) {
                this->init_parse_func_(parse_unsigned_t_<u_int8_t>);
                return true;
              }
            }
            this->init_parse_func_(parse_unsigned_t_<u_int32_t>);
            return true;
          }
        case SUBCLASS::TEMPERATURE:
          if (record_def->decimal_digits == DIGITS::D_2) {
            if (this->data_shift_ == 0) {
              if (this->data_mask_ == 0xFFFF) {
                this->init_parse_func_(parse_temperature_kelvin_<u_int16_t>);
                return true;
              }
              if (this->data_mask_ == 0xFF) {
                this->init_parse_func_(parse_temperature_kelvin_<u_int8_t>);
                return true;
              }
            }
            this->init_parse_func_(parse_temperature_kelvin_<u_int32_t>);
            return true;
          } else {
            this->init_parse_func_(parse_temperature_celsius_);
            return true;
          }
        case SUBCLASS::CELLVOLTAGE:
          this->init_parse_func_(parse_cell_voltage_);
          return true;
        default:
          return false;
      }

    case CLASS::ENUMERATION:
      if (this->data_shift_ == 0) {
        if (this->data_mask_ == 0xFF) {
          this->init_parse_func_(parse_bitmask_enum_t_<u_int8_t>);
          return true;
        }
        if (this->data_mask_ == 0xFFFF) {
          this->init_parse_func_(parse_bitmask_enum_t_<u_int16_t>);
          return true;
        }
      }
      this->init_parse_func_(parse_bitmask_enum_t_<u_int32_t>);
      return true;
    default:
      return false;
  }
}

template<typename T> void VBISensor::parse_bitmask_enum_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else
      static_cast<VBISensor *>(entity)->publish_state(value);
  }
}

template<typename T> void VBISensor::parse_signed_t_(VBIEntity *entity, const VBI_RECORD *record) {
  u_int32_t value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    auto sensor = static_cast<VBISensor *>(entity);
    if (value == sensor->nan_value_) {
      sensor->publish_state(NAN);
    } else if (value < sensor->nan_value_) {
      sensor->publish_state(value * sensor->scale_);
    } else {
      sensor->publish_state(((int32_t) value - sensor->signed_offset_) * sensor->scale_);
    }
  }
}

template<typename T> void VBISensor::parse_unsigned_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else {
      auto sensor = static_cast<VBISensor *>(entity);
      sensor->publish_state(value * sensor->scale_);
    }
  }
}

void VBISensor::parse_cell_voltage_(VBIEntity *entity, const VBI_RECORD *record) {
  u_int32_t value = entity->read_record_(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else {
      static_cast<VBISensor *>(entity)->publish_state((value + 260) * 0.01f);
    }
  }
}

void VBISensor::parse_temperature_celsius_(VBIEntity *entity, const VBI_RECORD *record) {
  u_int32_t value = entity->read_record_(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else {
      static_cast<VBISensor *>(entity)->publish_state((int) value - 40);
    }
  }
}

template<typename T> void VBISensor::parse_temperature_kelvin_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else {
      static_cast<VBISensor *>(entity)->publish_state(((int) value - 27315) * 0.01f);
    }
  }
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome