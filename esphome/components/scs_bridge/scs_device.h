#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace scs_bridge {

// base class for any SCS entity type (cover-switch-light-...)
class SCSDevice : public Component {
 public:
  SCSDevice() {}

  uint8_t get_address() { return this->address_; }
  void set_address(uint8_t address) { this->address_ = address; }

 protected:
  uint8_t address_{};
};

}  // namespace scs_bridge
}  // namespace esphome
