#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace scs_bridge {

// base class for any SCS entity type (cover-switch-light-...)
class SCSDevice : public Component {
 public:
  const uint8_t address;

  SCSDevice(uint8_t address) : address(address) {}

  // Component interface
  //void setup() override;
  //void loop() override;
  //void dump_config() override;

  //virtual void parse(uint8_t command, uint8_t value) = 0;

};

}  // namespace scs_bridge
}  // namespace esphome
