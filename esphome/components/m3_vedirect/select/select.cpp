#include "select.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "esphome/core/log.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "select";

void Select::parse_text_value(const char *text_value) {}
/*
void Select::parse_hex_value(const HexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value) && (this->state != hex_value)) {
    // copied code from Select::publish_state to optimize some inner behaviors (dynamic options fill)
    auto index = this->index_of(hex_value);
    size_t index_;
    if (index.has_value()) {
      index_ = index.value();
    } else {
      size_t index_ = this->traits_().options().size();
      this->traits_().options().push_back(hex_value);
      // TODO: check the size is consistent among different hex_values
      this->hex_data_size_ = hex_value.size() / 2;
    }
    this->has_state_ = true;
    this->state = hex_value;
    ESP_LOGD(TAG, "'%s': Sending state %s (index %zu)", this->get_name().c_str(), hex_value.c_str(), index_);
    this->state_callback_.call(hex_value, index_);
  }
}
*/
void Select::dynamic_register() {
  App.register_select(this);
  if (api::global_api_server) {
    add_on_state_callback([this](const std::string &state, size_t index) {
      api::global_api_server->on_select_update(this, state, index);
    });
  }
}

void Select::init_reg_def_(const REG_DEF *reg_def) {
  switch (reg_def->cls) {
    case REG_DEF::CLASS::ENUM:
      this->enum_def_ = reg_def->enum_def;
      for (auto &lookup_def : this->enum_def_->LOOKUPS) {
        this->traits_().options().push_back(std::string(lookup_def.label));
      }
      this->parse_hex_func_ = parse_hex_enum_;
      break;
    default:
      // defaults if nothing better
      this->parse_hex_func_ = parse_hex_default_;
      break;
  }
}

void Select::parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value)) {
    Select *select = static_cast<Select *>(entity);
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

void Select::parse_hex_enum_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  Select *select = static_cast<Select *>(entity);
  ENUM_DEF::data_type enum_value = hexframe->data_u8();
  if (select->enum_value_ != enum_value) {
    select->enum_value_ = enum_value;
    // the select::traits implementation is so bad...
    // it would be nice to have a data provider interface though but
    // this is it and we'd rather not patch the official esphome core.
    // Here we'll try to mantain sync between our enum_def and the select::options array
    // This code is safe as far as the enum_def->LOOKUPS is not modified by other parts
    // of the code
    auto &options = select->traits_().options();
    auto lookup_result = select->enum_def_->get_lookup(enum_value);
    if (lookup_result.added) {
      options.insert(options.begin() + lookup_result.index, std::string(lookup_result.lookup_def->label));
    }
    // Better safe than sorry..
    if (options.size() != select->enum_def_->LOOKUPS.size()) {
      options.clear();
      for (auto &lookup_def : select->enum_def_->LOOKUPS) {
        options.push_back(std::string(lookup_def.label));
      }
    }
    select->publish_state_(lookup_result.index);
  }
}

void Select::control(const std::string &value) {
  if (this->enum_def_) {
    auto lookup_def = this->enum_def_->lookup_value(value.c_str());
    if (lookup_def)
      this->manager->send_register_set(this->register_id_, lookup_def->value);
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
