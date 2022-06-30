#include "pipsolar_select.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "../pipsolar.h"

namespace esphome {
namespace m3_pipsolar {

static const char *const TAG = "m3_pipsolar.select";

void PipsolarSelect::publish_value(float value) {
  int i = 0;
  for (auto v: this->traits_values_) {
    if (v == value) {
      this->publish_state(this->traits.get_options()[i]);
      return;
    }
    ++i;
  }
}

void PipsolarSelect::control(const std::string &value) {

  int i = 0;
  for (auto trait: this->traits.get_options()) {
    if (trait == value) {
      char tmp[10];
      sprintf(tmp, this->set_command_, this->traits_values_[i]);
      ESP_LOGD(TAG, "Will write: %s out of value %s", tmp, value.c_str());
      this->parent_->switch_command(std::string(tmp));
      return;
    }
    ++i;
  }
}

}  // namespace m3_pipsolar
}  // namespace esphome
