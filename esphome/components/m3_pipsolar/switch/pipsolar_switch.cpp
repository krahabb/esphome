#include "pipsolar_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m3_pipsolar {

static const char *const TAG = "m3_pipsolar.switch";

void PipsolarSwitch::dump_config() { LOG_SWITCH("", "Pipsolar Switch", this); }
void PipsolarSwitch::write_state(bool state) {
  if (state) {
    if (this->on_command_.length() > 0) {
      this->parent_->switch_command(this->on_command_);
    }
  } else {
    if (this->off_command_.length() > 0) {
      this->parent_->switch_command(this->off_command_);
    }
  }
}

}  // namespace m3_pipsolar
}  // namespace esphome
