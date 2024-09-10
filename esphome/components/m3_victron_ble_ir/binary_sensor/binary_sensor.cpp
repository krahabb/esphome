#include "binary_sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"
namespace esphome {
namespace m3_victron_ble_ir {

VBIBinarySensor::VBIBinarySensor(TYPE type) : VBIEntity(type) {
  this->set_name(this->def->label);
  this->set_object_id(this->calculate_object_id_());
}

void VBIBinarySensor::init(const RECORD_DEF *record_def) {
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

template<typename T> void VBIBinarySensor::parse_bitmask_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    auto binary_sensor = static_cast<VBIBinarySensor *>(entity);
    binary_sensor->publish_state(value & binary_sensor->mask_);
  }
}

template<typename T> void VBIBinarySensor::parse_enum_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    auto binary_sensor = static_cast<VBIBinarySensor *>(entity);
    binary_sensor->publish_state(value == binary_sensor->mask_);
  }
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome