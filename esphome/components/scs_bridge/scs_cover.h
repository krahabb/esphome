#pragma once

#include "scs_device.h"
#include "esphome/components/cover/cover.h"
#include <string>

namespace esphome {
namespace scs_bridge {

class SCSCover : public SCSDevice, public cover::Cover {
 public:
  SCSCover(uint8_t address, std::string name);

  //config
  void set_max_duration(uint32_t seconds);

  //events from SCSBridge
  void command_up(uint32_t millis);
  void command_down(uint32_t millis);
  void command_stop(uint32_t millis);


  //Cover interface
  cover::CoverTraits get_traits() override;
  void control(const cover::CoverCall &call) override;

 protected:
  uint32_t command_millis_{0};//last 'active' received command
  int32_t position_millis_{0};//integral sum of movement time -> expresses position(timed)
  int32_t fullrun_millis_{30000};//duration of full movement

  // Scheduler callback to manage state publishing
  // while in transition
  void sch_refresh_();
  // Scheduler timeout to stop cover when we issue
  // a positioning command
  void sch_timeout_();
};

}//namespace scs_bridge
}//namespace esphome
