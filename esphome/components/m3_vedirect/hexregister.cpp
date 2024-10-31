#include "hexregister.h"
#include "manager.h"

namespace esphome {
namespace m3_vedirect {

const REG_DEF HexRegister::REG_DEF_UNDEFINED(0);
const TEXT_DEF HexRegister::TEXT_DEF_UNDEFINED;
void HexRegister::set_reg_def(Manager *manager, const REG_DEF *reg_def) {
  this->reg_def_ = reg_def;
  this->init_reg_def_();
  manager->hex_registers_.emplace(reg_def->register_id, this);
}

/*
void BitmaskHexRegister::link_disconnected_() { this->bitmask_ = BITMASK_DEF::VALUE_UNKNOWN; }
void BitmaskHexRegister::init_reg_def_() { this->parse_hex_ = parse_hex_bitmask_; }

void BitmaskHexRegister::parse_hex_bitmask_(HexRegister *hex_register, const RxHexFrame *hexframe) {
  BitmaskHexRegister *bitmask_hex_register = static_cast<BitmaskHexRegister *>(hex_register);
  BITMASK_DEF::bitmask_t bitmask =
      HEXFRAME::GET_DATA_AS_INT[bitmask_hex_register->reg_def_->data_type](hexframe->record());
  if (bitmask != bitmask_hex_register->bitmask_) {
    bitmask_hex_register->bitmask_ = bitmask;
    for (auto bitmask_parser : bitmask_hex_register->bitmask_parsers_) {
      bitmask_parser->parse_bitmask_(bitmask, bitmask_hex_register->reg_def_);
    }
  }
}
*/

}  // namespace m3_vedirect
}  // namespace esphome