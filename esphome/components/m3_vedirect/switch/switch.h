#pragma once
#include "esphome/components/switch/switch.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Switch final : public ConfigEntity, public Entity, public esphome::switch_::Switch {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Switch(Manager *manager) : ConfigEntity(manager), Entity(parse_hex_default_, parse_text_default_) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  Switch(Manager *manager) : ConfigEntity(manager), Entity(parse_hex_default_) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Switch(Manager *manager) : ConfigEntity(manager), Entity(parse_text_default_) {}
#endif

  static Entity *build_entity(Manager *manager, const char *name, const char *object_id);

  void set_mask(uint32_t mask) { this->mask_ = mask; }

 protected:
  friend class Manager;

  BITMASK_DEF::bitmask_t raw_value_{BITMASK_DEF::VALUE_UNKNOWN};
  BITMASK_DEF::bitmask_t mask_{0x01};

  void init_reg_def_() override;
  inline void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value) override;
  inline void parse_enum_(ENUM_DEF::enum_t enum_value) override;

  // interface esphome::switch_::Switch
#if defined(VEDIRECT_USE_HEXFRAME)
  void write_state(bool state) override;
#else
  void write_state(bool state) override{};
#endif

#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame);
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  static void parse_text_default_(HexRegister *hex_register, const char *text_value);
  static void parse_text_bitmask_(HexRegister *hex_register, const char *text_value);
  static void parse_text_enum_(HexRegister *hex_register, const char *text_value);
#endif
};

}  // namespace m3_vedirect
}  // namespace esphome
