#include "scs_switch.h"
#include "scs_bridge.h"
#include "esphome/core/log.h"
#include <string>

namespace esphome {
namespace scs_bridge {

SCSSwitch::SCSSwitch() : SCSDevice(), switch_::Switch() { SCSBridge::register_switch(this); }

void SCSSwitch::command_on(uint32_t micros) { this->publish_state(true); }

void SCSSwitch::command_off(uint32_t micros) { this->publish_state(false); }

void SCSSwitch::write_state(bool state) {
  SCSBridge::send(this->address_, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, state ? SCS_VAL_SWITCH_ON : SCS_VAL_SWITCH_OFF, true);
}

}  // namespace scs_bridge
}  // namespace esphome
