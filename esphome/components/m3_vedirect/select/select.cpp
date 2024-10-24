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
  if (reg_def) {
    switch (reg_def->cls) {
      case REG_DEF::CLASS::ENUM:
        this->enum_lookup_ = reg_def->enum_lookup;
        this->parse_hex_func_ = parse_hex_enum_;
        return;
      default:
        break;
    }
  }
  // defaults if nothing better
  this->parse_hex_func_ = parse_hex_default_;
}
void Select::parse_hex_default_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  std::string hex_value;
  if (hexframe->data_to_hex(hex_value)) {
    Select *select = static_cast<Select *>(entity);
    if (select->state != hex_value)
      select->publish_state(hex_value);
  }
}
void Select::parse_hex_enum_(VEDirectEntity *entity, const RxHexFrame *hexframe) {
  static_assert(RxHexFrame::ALLOCATED_DATA_SIZE >= 1, "HexFrame storage might lead to access overflow");
  Select *select = static_cast<Select *>(entity);
  ENUM_DEF::data_type enum_value = hexframe->data_u8();
  if (select->enum_value_ != enum_value) {
    select->enum_value_ = enum_value;
    auto enum_label = select->enum_lookup_(enum_value);
    if (enum_label)
      select->publish_state(std::string(enum_label));
    else
      select->publish_state(std::to_string(enum_value));
  }
}

void Select::control(const std::string &value) {
  // this->manager->send_register_set(this->register_id_, )
}

}  // namespace m3_vedirect
}  // namespace esphome
