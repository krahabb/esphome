#pragma once
#include "esphome/components/number/number.h"

#include "../entity.h"

namespace esphome {
namespace m3_vedirect {

class Number final : public ConfigEntity, public NumericEntity, public Entity, public esphome::number::Number {
 public:
#if defined(VEDIRECT_USE_HEXFRAME) && defined(VEDIRECT_USE_TEXTFRAME)
  Number(Manager *manager) : ConfigEntity(manager), Entity(parse_hex_default_, parse_text_empty_) {}
#elif defined(VEDIRECT_USE_HEXFRAME)
  Number(Manager *manager) : ConfigEntity(manager), Entity(parse_hex_default_) {}
#elif defined(VEDIRECT_USE_TEXTFRAME)
  Number(Manager *manager) : ConfigEntity(manager), Entity(parse_text_empty_) {}
#endif

  static Entity *build_entity(Manager *manager, const char *name, const char *object_id);

 protected:
  friend class Manager;
  void link_disconnected_() override;

  void init_reg_def_() override;

// interface esphome::number::Number
#if defined(VEDIRECT_USE_HEXFRAME)
  void control(float value) override;
  static void request_callback_(void *callback_param, const RxHexFrame *hex_frame);
#else
  void control(float value) override {}
#endif

#if defined(VEDIRECT_USE_HEXFRAME)
  static void parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static void parse_hex_temperature_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  template<typename T> static void parse_hex_t_(HexRegister *hex_register, const RxHexFrame *hex_frame);
  static const parse_hex_func_t DATA_TYPE_TO_PARSE_HEX_FUNC_[];
#endif
};

}  // namespace m3_vedirect
}  // namespace esphome
