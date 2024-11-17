#include "select.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include "esphome/core/log.h"
#include "../manager.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "select";

Entity *Select::build_entity(Manager *manager, const char *name, const char *object_id) {
  auto entity = new Select(manager);
  Entity::dynamic_init_entity_(entity, name, object_id, manager->get_vedirect_name(), manager->get_vedirect_id());
  App.register_select(entity);
  if (api::global_api_server)
    entity->add_on_state_callback([entity](const std::string &state, size_t index) {
      api::global_api_server->on_select_update(entity, state, index);
    });
  return entity;
}

void Select::init_reg_def_() {
  switch (this->reg_def_->cls) {
    case REG_DEF::CLASS::ENUM:
      for (auto &lookup_def : this->reg_def_->enum_def->LOOKUPS) {
        this->traits_().options().push_back(std::string(lookup_def.label));
      }
      this->parse_hex_ = parse_hex_enum_;
      this->parse_text_ = parse_text_enum_;
      break;
    default:
      break;
  }
}

void Select::parse_hex_default_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  char hex_value[RxHexFrame::ALLOCATED_ENCODED_SIZE];
  if (hex_frame->data_to_hex(hex_value, RxHexFrame::ALLOCATED_ENCODED_SIZE)) {
    static_cast<Select *>(hex_register)->parse_string_(hex_value);
  }
}

void Select::parse_hex_enum_(HexRegister *hex_register, const RxHexFrame *hex_frame) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  static_cast<Select *>(hex_register)->parse_enum_(hex_frame->data_t<ENUM_DEF::enum_t>());
}

void Select::parse_text_default_(HexRegister *hex_register, const char *text_value) {
  static_cast<Select *>(hex_register)->parse_string_(text_value);
}

void Select::parse_text_enum_(HexRegister *hex_register, const char *text_value) {
  char *endptr;
  ENUM_DEF::enum_t enum_value = strtoumax(text_value, &endptr, 0);
  if (*endptr == 0)
    static_cast<Select *>(hex_register)->parse_enum_(enum_value);
}

void Select::parse_enum_(ENUM_DEF::enum_t enum_value) {
  if (this->enum_value_ != enum_value) {
    this->enum_value_ = enum_value;
    // the select::traits implementation is so bad...
    // it would be nice to have a data provider interface though but
    // this is it and we'd rather not patch the official esphome core.
    // Here we'll try to mantain sync between our enum_def and the select::options array
    // This code is safe as far as the enum_def->LOOKUPS is not modified by other parts
    // of the code
    auto &options = this->traits_().options();
    auto enum_def = this->reg_def_->enum_def;
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
    this->publish_state_(lookup_result.index);
  }
}

void Select::parse_string_(const char *string_value) {
  if (strcmp(this->state.c_str(), string_value)) {
    auto &options = this->traits_().options();
    auto value = std::string(string_value);
    auto it = std::find(options.begin(), options.end(), value);
    auto index = std::distance(options.begin(), it);
    if (it == options.end()) {
      options.push_back(value);
    }
    this->publish_state_(index);
  }
}

void Select::control(const std::string &value) {
  auto lookup_def = this->reg_def_->enum_def->lookup_value(value.c_str());
  if (lookup_def)
    this->manager->send_register_set(this->reg_def_->register_id, lookup_def->value);
}

void Select::publish_state_(size_t index) {
  this->has_state_ = true;
  this->state = this->traits_().options()[index];
  ESP_LOGD(TAG, "'%s': Sending state %s (index %zu)", this->get_name().c_str(), this->state.c_str(), index);
  this->state_callback_.call(this->state, index);
}

}  // namespace m3_vedirect
}  // namespace esphome
