#include "select.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "esphome/core/log.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "select";

void Select::dynamic_register_() {
  App.register_select(this);
  if (api::global_api_server) {
    add_on_state_callback([this](const std::string &state, size_t index) {
      api::global_api_server->on_select_update(this, state, index);
    });
  }
}

void Select::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::ENUM:
      for (auto &lookup_def : this->reg_def_->enum_def->LOOKUPS) {
        this->traits_().options().push_back(std::string(lookup_def.label));
      }
      this->parse_hex_ = parse_hex_enum_;
      break;
    default:
      // defaults if nothing better
      this->parse_hex_ = parse_hex_default_;
      break;
  }
}

void Select::parse_hex_default_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value)) {
    Select *select = static_cast<Select *>(hexregister);
    if (select->state != hex_value) {
      auto &options = select->traits_().options();
      auto it = std::find(options.begin(), options.end(), hex_value);
      auto index = std::distance(options.begin(), it);
      if (it == options.end()) {
        options.push_back(hex_value);
      }
      select->publish_state_(index);
    }
  }
}

void Select::parse_hex_enum_(HexRegister *hexregister, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  Select *select = static_cast<Select *>(hexregister);
  ENUM_DEF::enum_type enum_value = hexframe->data_u8();
  if (select->enum_value_ != enum_value) {
    select->enum_value_ = enum_value;
    // the select::traits implementation is so bad...
    // it would be nice to have a data provider interface though but
    // this is it and we'd rather not patch the official esphome core.
    // Here we'll try to mantain sync between our enum_def and the select::options array
    // This code is safe as far as the enum_def->LOOKUPS is not modified by other parts
    // of the code
    auto &options = select->traits_().options();
    auto enum_def = select->reg_def_->enum_def;
    auto lookup_result = enum_def->get_lookup(enum_value);
    if (lookup_result.added) {
      options.insert(options.begin() + lookup_result.index, std::string(lookup_result.lookup_def->label));
    }
    // Better safe than sorry..
    if (options.size() != enum_def->LOOKUPS.size()) {
      options.clear();
      for (auto &lookup_def : enum_def->LOOKUPS) {
        options.push_back(std::string(lookup_def.label));
      }
    }
    select->publish_state_(lookup_result.index);
  }
}

void Select::control(const std::string &value) {
  if (this->reg_def_) {
    auto lookup_def = this->reg_def_->enum_def->lookup_value(value.c_str());
    if (lookup_def)
      this->manager->send_register_set(this->reg_def_->register_id, lookup_def->value);
  }
}

void Select::publish_state_(size_t index) {
  this->has_state_ = true;
  this->state = this->traits_().options()[index];
  ESP_LOGD(TAG, "'%s': Sending state %s (index %zu)", this->get_name().c_str(), this->state.c_str(), index);
  this->state_callback_.call(this->state, index);
}

}  // namespace m3_vedirect
}  // namespace esphome
