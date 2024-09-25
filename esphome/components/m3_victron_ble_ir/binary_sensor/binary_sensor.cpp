#include "binary_sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"
namespace esphome {
namespace m3_victron_ble_ir {

VBIBinarySensor::VBIBinarySensor(Manager *const manager, TYPE type) : VBIEntity(manager, type) {
  this->set_name(this->def.default_name);
  this->set_object_id(this->calculate_object_id_());
}

void VBIBinarySensor::link_disconnected() {
  // no way: BinarySensor cannot report unknown/disconnected
}

bool VBIBinarySensor::init_(const RECORD_DEF *record_def) {
  this->VBIEntity::init_(record_def);
  switch (this->def.subcls) {
    case SUBCLASS::ENUM:
    case SUBCLASS::SELECTOR:
      if (this->data_shift_ == 0) {
        if (this->data_mask_ == 0xFF) {
          this->init_parse_func_(parse_enum_t_<u_int8_t>);
          return true;
        }
        if (this->data_mask_ == 0xFFFF) {
          this->init_parse_func_(parse_enum_t_<u_int16_t>);
          return true;
        }
      }
      this->init_parse_func_(parse_enum_t_<u_int32_t>);
      return true;
    case SUBCLASS::BITMASK:
      if (this->data_shift_ == 0) {
        if (this->data_mask_ == 0xFF) {
          this->init_parse_func_(parse_bitmask_t_<u_int8_t>);
          return true;
        }
        if (this->data_mask_ == 0xFFFF) {
          this->init_parse_func_(parse_bitmask_t_<u_int16_t>);
          return true;
        }
      }
      this->init_parse_func_(parse_bitmask_t_<u_int32_t>);
      return true;
    default:
      return false;
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