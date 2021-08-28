#pragma once

#include "scs_device.h"
#include "esphome/components/switch/switch.h"
#include <string>

namespace esphome {
namespace scs_bridge {

class SCSSwitch : public SCSDevice, public switch_::Switch {
 public:
  SCSSwitch(uint8_t address, std::string name);

  //events from SCSBridge
  void command_on(uint32_t micros);
  void command_off(uint32_t micros);

protected:
  //Switch interface
  void write_state(bool state) override;

};

}//namespace scs_bridge
}//namespace esphome
