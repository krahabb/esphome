#pragma once

#include "esphome/components/select/select.h"
#include "esphome/core/component.h"

namespace esphome {
namespace m3_pipsolar {

class Pipsolar;

class PipsolarSelect : public select::Select {
 public:
  PipsolarSelect() {}
  void set_parent(Pipsolar *parent) { this->parent_ = parent; }
  void set_set_command(const char* command) { this->set_command_ = command; };  
  void set_traits_values(std::vector<float> traits_values) { this->traits_values_ = std::move(traits_values); }

  // overloads used by Pipsolar: convert intrinsic types to select::state
  void publish_value(float value);

 protected:
  void control(const std::string &value) override;

  const char* set_command_;
  Pipsolar *parent_;
  std::vector<float> traits_values_;
};

/*template<typename... Ts> class SetOutputAction : public Action<Ts...> {
 public:
  SetOutputAction(PipsolarOutput *output) : output_(output) {}

  TEMPLATABLE_VALUE(float, level)

  void play(Ts... x) override { this->output_->set_value(this->level_.value(x...)); }

 protected:
  PipsolarOutput *output_;
};*/

}  // namespace m3_pipsolar
}  // namespace esphome
