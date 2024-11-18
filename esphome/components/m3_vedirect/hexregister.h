#pragma once

#include "defines.h"
#include "ve_hexframe.h"

#include <vector>

namespace esphome {
namespace m3_vedirect {

class HexRegister {
 public:
  friend class Manager;
  friend class HexRegisterDispatcher;

  const REG_DEF *get_reg_def() { return this->reg_def_; }

  typedef FrameHandler::RxHexFrame RxHexFrame;
  typedef void (*parse_hex_func_t)(HexRegister *hex_register, const RxHexFrame *hexframe);
  inline void parse_hex(const RxHexFrame *hexframe) { this->parse_hex_(this, hexframe); }

  typedef void (*parse_text_func_t)(HexRegister *hex_register, const char *text_value);
  inline void parse_text(const char *text_value) { this->parse_text_(this, text_value); }

 protected:
  const REG_DEF *reg_def_;
  parse_hex_func_t parse_hex_;
  parse_text_func_t parse_text_;

  HexRegister(parse_hex_func_t parse_hex_func = parse_hex_empty_, parse_text_func_t parse_text_func = parse_text_empty_)
      : reg_def_(nullptr), parse_hex_(parse_hex_func), parse_text_(parse_text_func) {}

  // called by the Manager when VEDirect timeouts (we'll send 'unknown' to APIServer)
  virtual void link_disconnected_(){};
  /// @brief Preset entity properties based off our REG_DEF. This is being called
  /// automatically by components methods when a proper definition is available.
  /// @param reg_def: the proper register definition if available
  virtual void init_reg_def_(){};

  // Called by the manager to setup an HexRegisterDispatcher in order to cascade 'parse_hex' calls
  // when this HexRegister is being added to the registered registers. The base implementation will
  // setup a new HexRegisterDispatcher cascading this and the provided 'hex_register' while the
  // HexRegisterDispatcher will just add it to it's existing list
  virtual HexRegister *cascade_dispatcher_(HexRegister *hex_register);

  static void parse_hex_empty_(HexRegister *hex_register, const RxHexFrame *hexframe) {}

  static void parse_text_empty_(HexRegister *hex_register, const char *text_value) {}

  virtual void parse_bitmask_(BITMASK_DEF::bitmask_t bitmask_value){};
  virtual void parse_enum_(ENUM_DEF::enum_t enum_value){};
  virtual void parse_string_(const char *string_value){};
};

/// @brief This class provides hexframe dispatching to multiple HexRegisters when more than
/// one are interested in parsing incoming data for the same register address. This is
/// installed in the Manager.hex_registers_ collection in place of a single HexRegister so
/// that it'll be able to dispatch frame data to multiple entities/registers.
class HexRegisterDispatcher final : public HexRegister {
 public:
  friend class HexRegister;
  HexRegisterDispatcher() : HexRegister(parse_hex_default_, parse_text_empty_) {}

 protected:
  std::vector<HexRegister *> hex_registers_;

  void link_disconnected_() override {
    for (auto hex_register : this->hex_registers_) {
      hex_register->link_disconnected_();
    }
  }

  HexRegister *cascade_dispatcher_(HexRegister *hex_register) override {
    this->hex_registers_.push_back(hex_register);
    return this;
  }

  static void parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
    for (auto hex_register : static_cast<HexRegisterDispatcher *>(hex_register)->hex_registers_) {
      hex_register->parse_hex(hex_frame);
    }
  }
};

}  // namespace m3_vedirect
}  // namespace esphome