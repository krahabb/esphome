#include "binary_sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"
namespace esphome {
namespace m3_victron_ble_ir {

VBIBinarySensor::VBIBinarySensor(TYPE type) : VBIEntity(type) {
  this->set_name(this->def->label);
  this->set_object_id(str_sanitize(str_snake_case(this->get_name())).c_str());
}

void VBIBinarySensor::init_() {
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

template<typename T> void VBIBinarySensor::parse_bitmask_t_(VBIEntity *entity, const VICTRON_BLE_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
  }
}

template<typename T> void VBIBinarySensor::parse_enum_t_(VBIEntity *entity, const VICTRON_BLE_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
  }
}
}  // namespace m3_victron_ble_ir
}  // namespace esphome