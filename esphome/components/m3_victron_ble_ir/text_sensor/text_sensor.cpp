#include "text_sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"

namespace esphome {
namespace m3_victron_ble_ir {

VBITextSensor::VBITextSensor(Manager *const manager, TYPE type) : VBIEntity(manager, type) {
  this->set_name(this->def.default_name);
  this->set_object_id(this->calculate_object_id_());
}

void VBITextSensor::link_disconnected() {
  if (this->raw_value_ != this->nan_value_) {
    this->raw_value_ = this->nan_value_;
    this->publish_state("unknown");
  }
}

bool VBITextSensor::init_(const RECORD_DEF *record_def) {
  this->VBIEntity::init_(record_def);
  switch (this->def.subcls) {
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
    case SUBCLASS::ENUM:
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
    case SUBCLASS::SELECTOR:
      this->conditional_entities_ = new VBIEntity *[this->data_mask_ + 1] {};
      if (this->data_shift_ == 0) {
        if (this->data_mask_ == 0xFF) {
          this->init_parse_func_(parse_selector_t_<u_int8_t>);
          return true;
        }
        if (this->data_mask_ == 0xFFFF) {
          this->init_parse_func_(parse_selector_t_<u_int16_t>);
          return true;
        }
      }
      this->init_parse_func_(parse_selector_t_<u_int32_t>);
      return true;
    default:
      return false;
  }
}

template<typename T> void VBITextSensor::parse_bitmask_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value) {
      for (u_int32_t i = 1;; i = i << 1) {
        if (value & i) {
          static_cast<VBITextSensor *>(entity)->publish_state(entity->def.enum_lookup_func(i));
          break;
        }
      }
    } else {
      static_cast<VBITextSensor *>(entity)->publish_state(entity->def.enum_lookup_func(0));
    }
  }
}

template<typename T> void VBITextSensor::parse_enum_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    static_cast<VBITextSensor *>(entity)->publish_state(entity->def.enum_lookup_func(value));
  }
}

template<typename T> void VBITextSensor::parse_selector_t_(VBIEntity *entity, const VBI_RECORD *record) {
  auto textsensor = static_cast<VBITextSensor *>(entity);
  T value = entity->read_record_t_<T>(record);
  VBIEntity *selected_entity = textsensor->conditional_entities_[value];
  if (selected_entity)
    selected_entity->parse(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    textsensor->publish_state(entity->def.enum_lookup_func(value));
  }
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome