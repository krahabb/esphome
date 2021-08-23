#pragma once

#include "scs_device.h"
#include "esphome/components/cover/cover.h"
#include <string>

namespace esphome {
namespace scs_bridge {

class SCSCover : public SCSDevice, public cover::Cover {
 public:
  SCSCover(uint8_t address, std::string name);

  void command_up(uint32_t micros);
  void command_down(uint32_t micros);
  void command_stop(uint32_t micros);

  //low frequency call from SCSBridge::loop()
  void loop_refresh(uint32_t micros);


  //Cover interface
  cover::CoverTraits get_traits() override;
  void control(const cover::CoverCall &call) override;

 protected:
  uint32_t command_micros_;//last 'active' received command
  int32_t position_micros_;//integral sum of movement time -> expresses position(timed)
  int32_t fullrun_micros_;//duration of full movement
};

}//namespace scs_bridge
}//namespace esphome
