#include "manager.h"
#include "register.h"

#include "esphome/core/log.h"

namespace esphome {
namespace m3_vedirect {

static const char TAG[] = "m3_vedirect.%s";

const char *FRAME_ERRORS[Manager::Error::_COUNT] = {"None",          "Checksum",       "Coding",          "Overflow",
#if defined(VEDIRECT_USE_TEXTFRAME)
                                                    "NAME overflow", "VALUE overflow", "RECORD overflow",
#endif
                                                    "Timeout",       "Unexpected",     "Remote",          "Flags",
                                                    "Queue full"};

std::vector<Manager *> Manager::managers_;

const std::vector<Manager *> Manager::get_managers(const std::string &vedirect_id) {
  if (vedirect_id.empty()) {
    return {managers_.front()};
  } else if (vedirect_id == "*") {
    return managers_;
  } else {
    for (auto manager : managers_) {
      if (manager->vedirect_id_ == vedirect_id) {
        return {manager};
      }
    }
  }
  return {};
}

void Manager::setup() {
  Register::update_platforms();
  char *buf = new char[sizeof(TAG) + strlen(this->vedirect_id_)];
  sprintf(buf, TAG, this->vedirect_id_);
  this->logtag_ = buf;
#if defined(VEDIRECT_USE_HEXFRAME)
  this->last_ping_tx_ = -this->ping_timeout_;
#endif
  Manager::managers_.push_back(this);
}

void Manager::loop() {
  const int millis_ = millis();
  auto available = this->available();
  if (available) {
    uint8_t frame_buf[256];
    if (available > sizeof(frame_buf))
      available = sizeof(frame_buf);
    this->read_array(frame_buf, available);
    this->last_rx_ = millis_;
    this->decode(frame_buf, frame_buf + available);

#if defined(VEDIRECT_USE_HEXFRAME)
    if (this->ping_timeout_ && ((millis_ - this->last_ping_tx_) > this->ping_timeout_)) {
      this->request_command(HEXFRAME::COMMAND::Ping);
      this->last_ping_tx_ = millis_;
    }
#endif
  } else {
    if (this->connected_ && ((millis_ - this->last_frame_rx_) > VEDIRECT_LINK_TIMEOUT_MILLIS)) {
      this->on_disconnected_();
    }
  }

  // Checking requests timeouts
  if (auto request = this->requests_read_) {
    if (request->timeout < millis_) {
      this->request_response_(request, nullptr, Error::TIMEOUT);
    }
  }
}

void Manager::dump_config() { ESP_LOGCONFIG(this->logtag_, "VEDirect:"); }

void Manager::init_register(Register *reg, const REG_DEF *reg_def) {
  reg->reg_def_ = reg_def;
  reg->init_reg_def_();
  if (reg_def->register_id != REG_DEF::REGISTER_UNDEFINED) {
    auto result = this->hex_registers_.emplace(reg_def->register_id, reg);
    if (result.second) {
      // register_id already present in our set so we must setup/update a RegisterDispatcher
      auto &existing_pair = *result.first;
      existing_pair.second = existing_pair.second->cascade_dispatcher_(reg);
    }
  }
}

void Manager::init_register(Register *reg, REG_DEF::TYPE register_type) {
  this->init_register(reg, &REG_DEF::DEFS[register_type]);
#if defined(VEDIRECT_USE_TEXTFRAME)
  auto text_def = TEXT_DEF::find_type(register_type);
  if (text_def)
    this->emplace_text_register_(text_def->label, reg);
#endif
}

#if defined(VEDIRECT_USE_TEXTFRAME)
void Manager::init_register(Register *reg, const char *label) {
  auto text_def = TEXT_DEF::find_label(label);
  if (text_def) {
    if (!reg->reg_def_) {
      // only set reg_def from our presets (if any) if the yaml generated code
      // didn't set a custom configuration
      auto reg_def = REG_DEF::find_type(text_def->register_type);
      if (reg_def)
        this->init_register(reg, reg_def);
    }
  }
  this->emplace_text_register_(label, reg);
}
#endif  // defined(VEDIRECT_USE_TEXTFRAME)

Register *Manager::get_register(register_id_t register_id, bool auto_create) {
  auto entity_iter = this->hex_registers_.find(register_id);
  if (entity_iter == this->hex_registers_.end()) {
    if (auto_create) {
      // @todo: limit the number of auto-created registers since the new EspHome core
      // uses static pre-allocation for entities vectors and we have to manage this constraint
      // without setting up too large preallocations.
      ESP_LOGD(this->logtag_, "Auto-Creating HEX register: %04X", (int) register_id);
      Register *hex_register;
      auto reg_def = REG_DEF::find_register_id(register_id);
      if (reg_def) {
        switch (reg_def->cls) {
          case REG_DEF::CLASS::NUMERIC:
            if (reg_def->access == REG_DEF::ACCESS::READ_WRITE) {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Number](this, reg_def->label, reg_def->label);
            } else {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Sensor](this, reg_def->label, reg_def->label);
            }
            break;
          case REG_DEF::CLASS::BOOLEAN:
            if (reg_def->access == REG_DEF::ACCESS::READ_WRITE) {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Switch](this, reg_def->label, reg_def->label);
            } else {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::BinarySensor](this, reg_def->label, reg_def->label);
            }
            break;
          case REG_DEF::CLASS::ENUM:
            if (reg_def->access == REG_DEF::ACCESS::READ_WRITE) {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::Select](this, reg_def->label, reg_def->label);
            } else {
              hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, reg_def->label, reg_def->label);
            }
            break;
          case REG_DEF::CLASS::BITMASK: {
            hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, reg_def->label, reg_def->label);
          } break;
          default:
            hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, reg_def->label, reg_def->label);
        }
      } else {
        // else build a raw text sensor
        char *object_id = new char[7];
        sprintf(object_id, "0x%04X", (int) register_id);
        char *name = new char[16];
        sprintf(name, "Register %s", object_id);
        hex_register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, name, object_id);
        reg_def = new REG_DEF(register_id);
      }
      this->init_register(hex_register, reg_def);
      return hex_register;
    } else {
      return nullptr;
    }
  } else {
    return entity_iter->second;
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void Manager::send_hexframe(const HexFrame &hexframe) {
  this->write_array((const uint8_t *) hexframe.encoded(), hexframe.encoded_size());
  ESP_LOGD(this->logtag_, "HEX FRAME: sent %s", hexframe.encoded());
}

void Manager::send_hexframe(const char *rawframe, bool addchecksum) {
  HexFrameT<VEDIRECT_HEXFRAME_MAX_SIZE> hexframe;
  if (HexFrame::DecodeResult::Valid == hexframe.decode(rawframe, addchecksum)) {
    this->send_hexframe(hexframe);
  } else {
    ESP_LOGE(this->logtag_, "HEX FRAME: wrong encoding on request to send %s", rawframe);
  }
}

bool Manager::request(HEXFRAME::COMMAND command, register_id_t register_id, const void *data, size_t data_size,
                      request_callback_t &&callback) {
  if (this->is_request_queue_full()) {
    ESP_LOGW(this->logtag_, "HEX FRAME: queue full, dropping request (cmd '%01X' - reg '0x%04X')", command,
             register_id);
    if (callback) {
      callback(nullptr, Error::QUEUE_FULL);
    }
    return false;
  }
  ESP_LOGD(this->logtag_, "HEX FRAME: queuing request (cmd '%01X' - reg '0x%04X')", command, register_id);
  switch (command) {
    case HEXFRAME::COMMAND::Get:
      this->requests_write_->command_get(register_id);
      break;
    case HEXFRAME::COMMAND::Set:
      this->requests_write_->command_set(register_id, data, data_size);
      break;
    default:
      this->requests_write_->command(command);
      break;
  }
  this->requests_write_->callback = std::move(callback);
  if (!this->is_request_pending()) {
    this->request_trigger_(this->requests_write_);
  }
  if (this->requests_write_ == this->requests_last_)
    this->requests_write_ = this->requests_;
  else
    ++this->requests_write_;
  return true;
}
#endif  // defined(VEDIRECT_USE_HEXFRAME)

void Manager::on_connected_() {
  ESP_LOGD(this->logtag_, "LINK: connected");
  this->connected_ = true;
#if defined(VEDIRECT_USE_HEXFRAME)
  auto polling_size = this->hex_registers_.size();
  if (polling_size) {
    ESP_LOGD(this->logtag_, "Polling begin (%d registers)", polling_size);
    register_id_t *_polling_registers_end_;
    this->polling_registers_it_ = this->polling_registers_begin_ = _polling_registers_end_ =
        new register_id_t[polling_size];
    for (const auto &pair : this->hex_registers_) {
      *_polling_registers_end_++ = pair.first;
    }
    this->polling_registers_end_ = _polling_registers_end_;
    if (!this->is_request_pending()) {
      this->poll_next_register_();
    }  // else let the transaction management advance the polling
  }
#endif
#ifdef USE_BINARY_SENSOR
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(true);
  }
#endif
}

void Manager::on_disconnected_() {
  ESP_LOGD(this->logtag_, "LINK: disconnected");
  this->connected_ = false;
  this->reset();  // cleanup the frame handler

#if defined(VEDIRECT_USE_HEXFRAME)
  if (this->is_polling()) {
    ESP_LOGD(this->logtag_, "Polling cancelled");
    delete[] this->polling_registers_begin_;
    this->polling_registers_begin_ = this->polling_registers_end_ = nullptr;
  }
  if (auto request = this->requests_read_) {
    ESP_LOGD(this->logtag_, "Cancelling pending requests");
    for (; request != this->requests_write_;) {
      if (request->callback) {
        request->callback(nullptr, Error::TIMEOUT);
      }
      if (request == this->requests_last_)
        request = this->requests_;
      else
        ++request;
    }
    this->requests_read_ = nullptr;
  }
#endif

#ifdef USE_BINARY_SENSOR
  if (auto link_connected = this->link_connected_) {
    link_connected->publish_state(false);
  }
#endif
#if defined(VEDIRECT_USE_TEXTFRAME)
  for (auto &pair : this->text_registers_) {
    pair.second->link_disconnected_();
  }
#endif
  for (auto &pair : this->hex_registers_) {
    pair.second->link_disconnected_();
  }
}

#if defined(VEDIRECT_USE_HEXFRAME)
void Manager::request_trigger_(Request *request) {
  this->requests_read_ = request;
  request->timeout = millis() + VEDIRECT_COMMAND_TIMEOUT_MILLIS;
  this->write_array((const uint8_t *) request->encoded(), request->encoded_size());
}

void Manager::request_response_(Request *request, const HexFrame *response, Error error) {
// request is already valued with this->requests_read_
#if ESPHOME_LOG_LEVEL
  if (error) {
    ESP_LOGE(this->logtag_, "HEX FRAME: error {%s} on reply '%s' for request '%s'", FRAME_ERRORS[error],
             response ? response->encoded() : "", request->encoded());
  } else {
    ESP_LOGV(this->logtag_, "HEX FRAME: reply '%s' for request '%s'", response->encoded(), request->encoded());
  }
#endif
  if (request->callback) {
    request->callback(response, error);
  }
  if (request == this->requests_last_) {
    request = this->requests_;
  } else {
    ++request;
  }
  if (request == this->requests_write_) {
    // all requests processed
    this->requests_read_ = nullptr;
    if (this->is_polling()) {
      this->poll_next_register_();
    }
  } else {
    this->request_trigger_(request);
  }
}

void Manager::poll_next_register_() {
  // TODO: skip already updated registers
  this->request_get(*this->polling_registers_it_++, [this](const HexFrame *, uint8_t) {
    if (this->polling_registers_it_ == this->polling_registers_end_) {
      ESP_LOGD(this->logtag_, "Polling end");
      delete[] this->polling_registers_begin_;
      this->polling_registers_begin_ = this->polling_registers_end_ = nullptr;
    }
  });
}

void Manager::on_frame_hex_(const RxHexFrame &hexframe) {
  ESP_LOGD(this->logtag_, "HEX FRAME: received %s", hexframe.encoded());

  if (!this->connected_)
    this->on_connected_();

  this->last_frame_rx_ = this->last_rx_;
  this->hexframe_callback_.call(hexframe);

#ifdef USE_TEXT_SENSOR
  if (this->rawhexframe_)
    this->rawhexframe_->publish_state(std::string(hexframe.encoded()));
#endif

  Request *request;

  auto rx_command = hexframe.command();
  if (rx_command == HEXFRAME::COMMAND::Async) {
    goto _forward_to_register;
  }

  if (request = this->requests_read_) {
    switch (rx_command) {
      case HEXFRAME::COMMAND::Get:
      case HEXFRAME::COMMAND::Set:
        this->request_response_(request, &hexframe,
                                (request->command() != rx_command) || (request->register_id() != hexframe.register_id())
                                    ? Error::UNEXPECTED
                                    : (hexframe.flags() ? Error::FLAGS : Error::NONE));
        goto _forward_to_register;
      case HEXFRAME::COMMAND::PingResp:
        this->request_response_(request, &hexframe,
                                request->command() != HEXFRAME::COMMAND::Ping ? Error::UNEXPECTED : Error::NONE);
        return;
      case HEXFRAME::COMMAND::Done:
        this->request_response_(request, &hexframe, Error::NONE);
        return;
      // case HEXFRAME::COMMAND::Error:
      // case HEXFRAME::COMMAND::Unknown:
      default:
        this->request_response_(request, &hexframe, Error::REMOTE);
        return;
    }
  } else {
    ESP_LOGE(this->logtag_, "HEX FRAME: unexpected frame (no requests pending)");
    switch (rx_command) {
      case HEXFRAME::COMMAND::Get:
      case HEXFRAME::COMMAND::Set:
        goto _forward_to_register;
      default:
        return;
    }
  }

_forward_to_register:
  if (hexframe.data_size() > 0) {
    Register *hex_register = this->get_register(hexframe.register_id(), this->auto_create_hex_entities_);
    if (hex_register)
      hex_register->parse_hex(&hexframe);
  } else {
    ESP_LOGE(this->logtag_, "HEX FRAME: inconsistent size: %s", hexframe.encoded());
  }
}

void Manager::on_frame_hex_error_(FrameHandler::Error error) {
  if (this->requests_read_) {
    this->request_response_(this->requests_read_, nullptr, static_cast<Error>(error));
  } else {
    ESP_LOGE(this->logtag_, "HEX FRAME: unexpected error {%s} (no requests pending)", FRAME_ERRORS[error]);
  }
}
#endif  // #if defined(VEDIRECT_USE_HEXFRAME)

#if defined(VEDIRECT_USE_TEXTFRAME)
void Manager::emplace_text_register_(const char *label, Register *_register) {
  auto result = this->text_registers_.emplace(label, _register);
  if (!result.second) {
    // label already present in our set so we must setup/update a RegisterDispatcher
    auto &existing_pair = *result.first;
    existing_pair.second = existing_pair.second->cascade_dispatcher_(_register);
  }
}

void Manager::on_frame_text_(TextRecord **text_records, uint8_t text_records_count) {
  ESP_LOGD(this->logtag_, "TEXT FRAME: processing");

  if (!this->connected_)
    this->on_connected_();

  this->last_frame_rx_ = this->last_rx_;

#ifdef USE_TEXT_SENSOR
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
#endif

  for (uint8_t i = 0; i < text_records_count; ++i) {
    const TextRecord *text_record = text_records[i];
    auto entity_iter = this->text_registers_.find(text_record->name);
    if (entity_iter == this->text_registers_.end()) {
      if (this->auto_create_text_entities_) {
        ESP_LOGD(this->logtag_, "Auto-Creating TEXT register: %s", text_record->name);
        Register *_register;
        const char *label;
        auto text_def = TEXT_DEF::find_label(text_record->name);
        if (text_def) {
          label = text_def->label;
          // check if we have an already defined matching hex register
          auto reg_def = REG_DEF::find_type(text_def->register_type);
          if (reg_def) {
            _register = this->get_register(reg_def->register_id, true);
          } else {
            _register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, label, label);
          }
        } else {
          // We lack the definition for this TEXT RECORD so
          // we return a plain TextSensor entity.
          // We allocate a copy since the label param is 'volatile'
          label = strdup(text_record->name);
          _register = Register::BUILD_ENTITY_FUNC[Register::TextSensor](this, label, label);
        }
        // no need to check for cascading -> emplace straight
        this->text_registers_.emplace(label, _register);
        _register->parse_text(text_record->value);
      }
    } else {
      entity_iter->second->parse_text(text_record->value);
    }
  }
}

void Manager::on_frame_text_error_(FrameHandler::Error error) {
  ESP_LOGE(this->logtag_, "TEXT FRAME: %s", FRAME_ERRORS[error]);
}
#endif  // #if defined(VEDIRECT_USE_TEXTFRAME)

}  // namespace m3_vedirect
}  // namespace esphome
