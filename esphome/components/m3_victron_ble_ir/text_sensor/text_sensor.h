#pragma once

#include "esphome/core/log.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "../entity.h"
namespace esphome {
namespace m3_victron_ble_ir {

class VBITextSensor : public VBIEntity, public text_sensor::TextSensor {
 public:
  VBITextSensor(Manager *const manager, TYPE type);

  bool is_text_sensor() override { return true; }

  void link_disconnected() override;

  bool register_conditional_entity(VBIEntity *entity, u_int8_t selector_value) {
    // play it safe since the entity might not be (correctly) initialized
    if (this->conditional_entities_) {
      // no check on index bounds here..
      this->conditional_entities_[selector_value] = entity;
      return true;
    }
    return false;
  }

 protected:
  VBIEntity **conditional_entities_{};

  bool init_(const RECORD_DEF *record_def) override;

  template<typename T> static void parse_bitmask_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_enum_t_(VBIEntity *entity, const VBI_RECORD *record);
  template<typename T> static void parse_selector_t_(VBIEntity *entity, const VBI_RECORD *record);
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome