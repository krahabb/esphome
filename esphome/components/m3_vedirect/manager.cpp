#include "manager.h"
#include "esphome/core/log.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/components/uart/uart_component.h"
// #include <esphome/components/deep_sleep/deep_sleep_component.h>

#ifdef USE_ESP32
#include "esp_pm.h"
#include <esp_sleep.h>
#include <esp_wifi.h>
#endif

#include "factory.h"
#include "entity.h"
#include "hexframe.h"

namespace esphome {
namespace m3_vedirect {

static const char TAG[] = "m3_vedirect.%s";

std::vector<Manager *> Manager::managers_;

void Manager::setup() {
  char *buf = new char[sizeof(TAG) + this->vedirect_id_.length()];
  sprintf(buf, TAG, this->vedirect_id_.c_str());
  this->logtag_ = buf;

  Manager::managers_.push_back(this);
}

void Manager::loop() {
  const uint32_t millis_ = millis();

  if (this->run_time_) {
    float run_time = millis_ / 1000;
    if (run_time != this->run_time_->raw_state)
      this->run_time_->publish_state(run_time);
  }

  auto available = this->available();
  if (!available) {
    if (this->connected_ && ((millis_ - this->millis_last_rx_) > VEDIRECT_TIMEOUT_MILLIS)) {
      this->on_disconnected_();
    }
    return;
  }

  uint8_t frame_buf[256];
  if (available > sizeof(frame_buf))
    available = sizeof(frame_buf);
  if (this->read_array(frame_buf, available)) {
    this->millis_last_rx_ = millis_;
    this->decode(frame_buf, frame_buf + available);
  }

  if (this->ping_retry_timeout_ && ((millis_ - this->millis_last_ping_tx_) > this->ping_retry_timeout_)) {
    this->send_hexframe(HexFrame_Command(HexFrame::Ping));
    this->millis_last_ping_tx_ = this->millis_last_hexframe_tx_;
  }
}

void Manager::dump_config() { ESP_LOGCONFIG(this->logtag_, "VEDirect:"); }

Manager *Manager::get_manager(const std::string &vedirect_id) {
  if (vedirect_id.empty()) {
    return managers_.front();
  } else {
    for (auto manager : managers_) {
      if (manager->vedirect_id_ == vedirect_id) {
        return manager;
      }
    }
  }
  return nullptr;
}

void Manager::setup_entity_name_id(EntityBase *entity, const char *name, const char *object_id) {
  // set_name before set_object_id else it will fckup object_id generation
  if (this->vedirect_name_.empty()) {
    entity->set_name(name);
  } else {
    char *entity_name = new char[this->vedirect_name_.size() + strlen(name) + 2];
    sprintf(entity_name, "%s.%s", this->vedirect_name_.c_str(), name);
    entity->set_name(entity_name);
  }

  char *entity_object_id = new char[this->vedirect_id_.size() + strlen(object_id) + 2];
  sprintf(entity_object_id, "%s.%s", this->vedirect_id_.c_str(), object_id);
  entity->set_object_id(entity_object_id);
}

void Manager::send_hexframe(const HexFrame &hexframe) {
  this->write_array((const uint8_t *) hexframe.encoded(), hexframe.encoded_size());
  this->millis_last_hexframe_tx_ = millis();
  ESP_LOGD(this->logtag_, "HEX FRAME: sent %s", hexframe.encoded());
}

void Manager::send_hexframe(const char *rawframe, bool addchecksum) {
  HexFrameT<VEDIRECT_HEXFRAME_SIZE> hexframe;
  if (HexFrame::DecodeResult::Valid == hexframe.decode(rawframe, addchecksum)) {
    this->send_hexframe(hexframe);
  } else {
    ESP_LOGE(this->logtag_, "HEX FRAME: wrong encoding on request to send %s", rawframe);
  }
}

/* void Manager::send_hexframe(const std::string &vedirect_id, const std::string &payload) {
  if (vedirect_id.empty()) {
    managers_.front()->send_hexframe(payload.c_str(), false);
  } else {
    for (auto manager : managers_) {
      if (manager->vedirect_id_ == vedirect_id) {
        manager->send_hexframe(payload.c_str(), false);
        break;
      }
    }
  }
}*/

void Manager::on_connected_() {
  ESP_LOGD(this->logtag_, "LINK: connected");
  this->connected_ = true;
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(true);
  }
  if (this->auto_create_hex_entities_ || this->hex_registers_.size()) {
    this->send_hexframe(HexFrame_Command(HexFrame::Ping));
    this->millis_last_ping_tx_ = this->millis_last_hexframe_tx_;
  }
}

void Manager::on_disconnected_() {
  ESP_LOGD(this->logtag_, "LINK: disconnected");
  this->connected_ = false;
  this->reset();  // cleanup the frame handler
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(false);
  }

  // we should set all entities state to HA 'unavailable' but
  // ESPHOME right now doesn't support it
}
void Manager::on_frame_hex_(const HexFrame &hexframe) {
  ESP_LOGD(this->logtag_, "HEX FRAME: received %s", hexframe.encoded());

  if (!this->connected_)
    this->on_connected_();

  this->hexframe_callback_.call(hexframe);

  if (this->rawhexframe_)
    this->rawhexframe_->publish_state(std::string(hexframe.encoded()));

  this->millis_last_hexframe_rx_ = this->millis_last_rx_;
  switch (hexframe.command()) {
    case HexFrame::Command::Get:
    case HexFrame::Command::Set:
    case HexFrame::Command::Async: {
      if (hexframe.data_size() > 0) {
        register_id_t hex_id = hexframe.register_id();
        auto entity_iter = this->hex_registers_.find(hex_id);
        if (entity_iter == this->hex_registers_.end()) {
          if (this->auto_create_hex_entities_) {
            ESP_LOGD(this->logtag_, "Looking-up entity for VE.Direct hex register: %04X", (int) hex_id);
            auto hex_register = Factory::build_register(this, hex_id);
            hex_register->dynamic_register();
            hex_register->parse_hex_value(&hexframe);
          }
        } else {
          entity_iter->second->parse_hex_value(&hexframe);
        }
      } else {
        ESP_LOGE(this->logtag_, "Inconsistent hex frame size: %s", hexframe.encoded());
      }
    }
  }
}

void Manager::on_frame_text_(TextRecord **text_records, uint8_t text_records_count) {
  ESP_LOGD(this->logtag_, "TEXT FRAME: processing");

  if (!this->connected_)
    this->on_connected_();

  this->millis_last_textframe_rx_ = this->millis_last_rx_;

  if (auto rawtextframe = this->rawtextframe_) {
    std::string textframe_value;
    textframe_value.reserve(text_records_count * sizeof(FrameHandler::TextRecord));
    for (uint8_t i = 0; i < text_records_count; ++i) {
      const TextRecord *text_record = text_records[i];
      textframe_value.append(text_record->name);
      textframe_value.append(":");
      textframe_value.append(text_record->value);
      textframe_value.append(",");
    }
    if (rawtextframe->raw_state != textframe_value) {
      rawtextframe->publish_state(textframe_value);
    }
  }

  for (uint8_t i = 0; i < text_records_count; ++i) {
    const TextRecord *text_record = text_records[i];
    auto entity_iter = this->text_entities_.find(text_record->name);
    if (entity_iter == this->text_entities_.end()) {
      if (this->auto_create_text_entities_) {
        ESP_LOGD(this->logtag_, "Looking-up entity for VE.Direct text field: %s", text_record->name);
        auto entity = TFEntity::build(this, text_record->name);
        entity->dynamic_register();
        entity->parse_text_value(text_record->value);
      }
    } else {
      entity_iter->second->parse_text_value(text_record->value);
    }
  }
}

void Manager::on_frame_error_(const char *message) { ESP_LOGE(this->logtag_, message); }

}  // namespace m3_vedirect
}  // namespace esphome
