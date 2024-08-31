#include "text_sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"

namespace esphome {
namespace m3_victron_ble_ir {

// static const char *const TAG = "m3_victron_ble_ir.text_sensor";

void VBITextSensor::init_() {
  switch (this->def->cls) {
    case CLASS::ENUM:
      this->raw_value_ = this->get_record_def()->data_mask;
      if ((this->get_record_def()->data_shift == 0) && (this->get_record_def()->data_mask == 0xFF)) {
        this->set_parse_func_(parse_enum_8bit_aligned_);
      } else {
        this->set_parse_func_(parse_enum_);
      }
      break;
    case CLASS::BITMASK:
      this->raw_value_ = this->get_record_def()->data_mask;
      this->set_parse_func_(parse_bitmask_);
      break;
    default:
      this->init_unsupported_();
  }
}

void VBITextSensor::parse_enum_(VBIEntity *entity, const VICTRON_BLE_RECORD *record) {
  VBITextSensor *text_sensor = static_cast<VBITextSensor *>(entity);
  u_int32_t value = RECORD_DEF::get_u_int32_t(entity->get_record_def(), record);
  if (value != text_sensor->raw_value_) {
    text_sensor->raw_value_ = value;
    text_sensor->publish_state(((EnumBase::lookup_func_t) text_sensor->def->params)(value));
  }
}

void VBITextSensor::parse_enum_8bit_aligned_(VBIEntity *entity, const VICTRON_BLE_RECORD *record) {
  VBITextSensor *text_sensor = static_cast<VBITextSensor *>(entity);
  u_int32_t value = RECORD_DEF::get_u_int8_t_aligned(entity->get_record_def(), record);
  if (value != text_sensor->raw_value_) {
    text_sensor->raw_value_ = value;
    text_sensor->publish_state(((EnumBase::lookup_func_t) text_sensor->def->params)(value));
  }
}

void VBITextSensor::parse_bitmask_(VBIEntity *entity, const VICTRON_BLE_RECORD *record) {
  VBITextSensor *text_sensor = static_cast<VBITextSensor *>(entity);
  auto record_def = entity->get_record_def();
  u_int32_t value = RECORD_DEF::get_u_int32_t(record_def, record);
  if (value != text_sensor->raw_value_) {
    text_sensor->raw_value_ = value;
    if (value) {
      for (u_int32_t i = 1;; i = i << 1) {
        if (value & i) {
          text_sensor->publish_state(((EnumBase::lookup_func_t) text_sensor->def->params)(i));
          break;
        }
      }
    } else {
      text_sensor->publish_state(((EnumBase::lookup_func_t) text_sensor->def->params)(0));
    }
  }
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome