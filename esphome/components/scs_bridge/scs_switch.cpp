#include "scs_switch.h"
#include "scs_bridge.h"
#include "esphome/core/log.h"
#include <string>

namespace esphome {
namespace scs_bridge {

SCSSwitch::SCSSwitch(uint8_t address, std::string name)
    : SCSDevice(address), switch_::Switch(name) {
  ESP_LOGCONFIG(SCSBridge::TAG, "SCSSwitch::SCSSwitch address=%02X name=%s", (uint32_t)address, name.c_str());
  SCSBridge::register_switch(this);
}

void SCSSwitch::command_on(uint32_t micros) { this->publish_state(true); }

void SCSSwitch::command_off(uint32_t micros) { this->publish_state(false); }

void SCSSwitch::write_state(bool state) {
  SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, state ? SCS_VAL_SWITCH_ON : SCS_VAL_SWITCH_OFF, true);
}

}//namespace scs_bridge
}//namespace esphome
