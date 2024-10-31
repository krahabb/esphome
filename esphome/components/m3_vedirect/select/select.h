#pragma once
#include "esphome/components/select/select.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Select final : public ConfigEntity, public esphome::select::Select {
 public:
  Select(Manager *manager) : ConfigEntity(manager) {}

 protected:
  friend class Manager;
  ENUM_DEF::enum_t enum_value_{0xFF};

  void dynamic_register_() override;
  void init_reg_def_() override;

  static void parse_hex_default_(HexRegister *hexregister, const RxHexFrame *hexframe);
  static void parse_hex_enum_(HexRegister *hexregister, const RxHexFrame *hexframe);

  void init_text_def_(const TEXT_DEF *text_def) override;
  static void parse_text_enum_(HexRegister *hex_register, const char *text_value);

  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override;

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
