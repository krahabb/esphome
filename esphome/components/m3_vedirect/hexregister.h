#pragma once

#include "defines.h"
#include "ve_hexframe.h"

namespace esphome {
namespace m3_vedirect {

class HexRegister {
 public:
  void set_reg_def(Manager *manager, const REG_DEF *reg_def);
  const REG_DEF *get_reg_def() { return this->reg_def_; }

  typedef FrameHandler::RxHexFrame RxHexFrame;
  typedef void (*parse_hex_func_t)(HexRegister *hex_register, const RxHexFrame *hexframe);
  inline void parse_hex(const RxHexFrame *hexframe) { this->parse_hex_(this, hexframe); }

  typedef void (*parse_text_func_t)(HexRegister *hex_register, const char *text_value);
  inline void parse_text(const char *text_value) { this->parse_text_(this, text_value); }

 protected:
  friend class Manager;
  static const REG_DEF REG_DEF_UNDEFINED;
  const REG_DEF *reg_def_{&REG_DEF_UNDEFINED};
  parse_hex_func_t parse_hex_{parse_hex_empty_};

  // called by the Manager when VEDirect timeouts (we'll send 'unknown' to APIServer)
  virtual void link_disconnected_(){};
  /// @brief Preset entity properties based off our REG_DEF. This is being called
  /// automatically by components methods when a proper definition is available.
  /// @param reg_def: the proper register definition if available
  virtual void init_reg_def_(){};

  static void parse_hex_empty_(HexRegister *hex_register, const RxHexFrame *hexframe) {}

  static const TEXT_DEF TEXT_DEF_UNDEFINED;
  parse_text_func_t parse_text_{parse_text_empty_};
  /// @brief Preset entity properties based off our TEXT_DEF. This is being called
  /// automatically when a proper definition is available.
  virtual void init_text_def_(const TEXT_DEF *text_def) {}

  static void parse_text_empty_(HexRegister *hex_register, const char *text_value) {}

  virtual void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value){};
  virtual void parse_enum_(ENUM_DEF::enum_t enum_value){};
  virtual void parse_string_(const char *string_value){};
};

/*
/// @brief Base class (interface) for entities which are linked to BITMASK registers
/// like BinarySensor, TextSensor, Switch
class BitmaskParser {
 protected:
  friend class BitmaskHexRegister;
  virtual void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask, const REG_DEF *reg_def) {};
};

class BitmaskHexRegister : public HexRegister {
 public:
  void register_bitmask_parser(BitmaskParser *bitmask_parser) { this->bitmask_parsers_.push_back(bitmask_parser); }

 protected:
  BITMASK_DEF::bitmask_t bitmask_{BITMASK_DEF::VALUE_UNKNOWN};
  std::vector<BitmaskParser *> bitmask_parsers_;

  void link_disconnected_() override;
  void init_reg_def_() override;

  static void parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hexframe);
};
*/

}  // namespace m3_vedirect
}  // namespace esphome