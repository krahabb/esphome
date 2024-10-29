#pragma once

#include "defines.h"
#include "ve_hexframe.h"

namespace esphome {
namespace m3_vedirect {

class HexRegister {
 public:
  void set_reg_def(const REG_DEF *reg_def) {
    this->reg_def_ = reg_def;
    this->init_reg_def_();
  }
  const REG_DEF *get_reg_def() { return this->reg_def_; }

  typedef FrameHandler::RxHexFrame RxHexFrame;
  typedef void (*parse_hex_func_t)(HexRegister *hexregister, const RxHexFrame *hexframe);
  inline parse_hex_func_t parse_hex() { return this->parse_hex_; }

 protected:
  friend class Manager;
  const REG_DEF *reg_def_{};
  parse_hex_func_t parse_hex_{parse_hex_empty_};

  // called by the Manager when VEDirect timeouts (we'll send 'unknown' to APIServer)
  virtual void link_disconnected_(){};
  /// @brief Preset entity properties based off our REG_DEF. This is being called
  /// automatically by components methods when a proper definition is available.
  /// @param reg_def: the proper register definition if available
  virtual void init_reg_def_(){};

  static void parse_hex_empty_(HexRegister *hexregister, const RxHexFrame *hexframe) {}
};

/// @brief Base class (interface) for entities which are linked to BITMASK registers
/// like BinarySensor, TextSensor, Switch
class BitmaskParser {
 protected:
  friend class BitmaskHexRegister;
  virtual void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask, const REG_DEF *reg_def){};
};

class BitmaskHexRegister : public HexRegister {
 public:
  void register_bitmask_parser(BitmaskParser *bitmask_parser) { this->bitmask_parsers_.push_back(bitmask_parser); }

 protected:
  BITMASK_DEF::bitmask_t bitmask_{BITMASK_DEF::VALUE_UNKNOWN};
  std::vector<BitmaskParser *> bitmask_parsers_;

  void link_disconnected_() override;
  void init_reg_def_() override;

  static void parse_hex_bitmask_(HexRegister *hexregister, const RxHexFrame *hexframe);
};

}  // namespace m3_vedirect
}  // namespace esphome