#pragma once
#include "esphome/components/select/select.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Select : public esphome::select::Select, public VEDirectEntity {
 public:
  Select(Manager *manager) : VEDirectEntity(manager) {}

  // interface VEDirectEntity

  void parse_text_value(const char *text_value) override;

  void dynamic_register() override;

 protected:
  ENUM_DEF::data_type enum_value_{0xFF};
  ENUM_DEF *enum_def_{};

  void init_reg_def_(const REG_DEF *reg_def) override;
  static void parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe);
  static void parse_hex_enum_(VEDirectEntity *entity, const RxHexFrame *hexframe);

  // interface esphome::select::Select

  void control(const std::string &value) override;

  // Hack the basic SelectTraits to allow dynamic management
  // of options from our Select entity without always copying/moving
  class SelectTraits : public esphome::select::SelectTraits {
   public:
    inline std::vector<std::string> &options() { return this->options_; }
  };

  inline SelectTraits &traits_() { return reinterpret_cast<SelectTraits &>(this->traits); }

  // 'optimized' publish_state bypassing index checks since we're mantaining our
  // own 'source of truth' in enum_def_
  void publish_state_(size_t index);
};

}  // namespace m3_vedirect
}  // namespace esphome
