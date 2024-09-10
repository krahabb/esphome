#include "text_sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"

namespace esphome {
namespace m3_victron_ble_ir {

// static const char *const TAG = "m3_victron_ble_ir.text_sensor";
VBITextSensor::VBITextSensor(TYPE type) : VBIEntity(type) {
  this->set_name(this->def->label);
  this->set_object_id(this->calculate_object_id_());
}

void VBITextSensor::init(const RECORD_DEF *record_def) {
  this->VBIEntity::init(record_def);
  switch (this->def->cls) {
    case CLASS::ENUM:
      if (this->data_shift_ == 0) {
        if (this->data_mask_ == 0xFF) {
          this->set_parse_func_(parse_enum_t_<u_int8_t>);
          break;
        }
        if (this->data_mask_ == 0xFFFF) {
          this->set_parse_func_(parse_enum_t_<u_int16_t>);
          break;
        }
      }
      this->set_parse_func_(parse_enum_t_<u_int32_t>);
      break;
    case CLASS::BITMASK:
      if (this->data_shift_ == 0) {
        if (this->data_mask_ == 0xFF) {
          this->set_parse_func_(parse_bitmask_t_<u_int8_t>);
          break;
        }
        if (this->data_mask_ == 0xFFFF) {
          this->set_parse_func_(parse_bitmask_t_<u_int16_t>);
          break;
        }
      }
      this->set_parse_func_(parse_bitmask_t_<u_int32_t>);
      break;
    default:
      this->init_unsupported_();
  }
}

template<typename T> void VBITextSensor::parse_bitmask_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value) {
      for (u_int32_t i = 1;; i = i << 1) {
        if (value & i) {
          static_cast<VBITextSensor *>(entity)->publish_state(entity->def->enum_lookup_func(i));
          break;
        }
      }
    } else {
      static_cast<VBITextSensor *>(entity)->publish_state(entity->def->enum_lookup_func(0));
    }
  }
}

template<typename T> void VBITextSensor::parse_enum_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    static_cast<VBITextSensor *>(entity)->publish_state(entity->def->enum_lookup_func(value));
  }
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome