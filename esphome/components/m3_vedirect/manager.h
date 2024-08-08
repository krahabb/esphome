#pragma once
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include <unordered_map>
#include <string_view>

namespace esphome {
namespace m3_vedirect {

#ifdef ESPHOME_LOG_HAS_VERBOSE
// enables detailed state machine logging (sends to ESP_LOGV macro)
// #define VEDIRECT_DEBUG_STATE
#endif

// maximum amount of time (millis) without receiving data
// after which we consider the vedirect link disconnected
#define VEDIRECT_TIMEOUT_MILLIS 5000

#define VEDIRECT_NAME_LEN 9
#define VEDIRECT_VALUE_LEN 33
#define VEDIRECT_RECORDS_COUNT 22
// despite the expectation of having maximum 22 fields of max 45 chars
// I guess long fields are rare and Victron implements a maximum frame size this way
// splitting longer list of fields among multiple text frames
#define VEDIRECT_BUFFER_LEN 512

class HexRxFrame;
class HexTxFrame;
class HexRegister;
class TFEntity;

#define MANAGER_ENTITY_(type, name) \
 protected: \
  type *name##_{}; /* NOLINT */ \
\
 public: \
  void set_##name(type *name) { /* NOLINT */ \
    this->name##_ = name; \
  }

#if __cpp_constexpr >= 201304L
#define _RELAXEDCONSTEXPR constexpr
#else
#define _RELAXEDCONSTEXPR
#endif
#define _HASH_SHIFT 16u
#define _HASH_MUL 23456789u

struct cstring_hash {
  _RELAXEDCONSTEXPR size_t operator()(const char *s) const {
    size_t h = 0;
    for (; *s; ++s) {
      h = h * _HASH_MUL + static_cast<unsigned char>(*s);
      h ^= h >> _HASH_SHIFT;
    }
    return h *= _HASH_MUL;
  }
};

struct cstring_eq {
  _GLIBCXX14_CONSTEXPR
  bool operator()(const char *__x, const char *__y) const { return !strcmp(__x, __y); }
};

class Manager : public uart::UARTDevice, public Component {
  // dedicated entities to manage component state/behavior
  MANAGER_ENTITY_(text_sensor::TextSensor, rawhexframe)
  MANAGER_ENTITY_(text_sensor::TextSensor, rawtextframe)
  MANAGER_ENTITY_(binary_sensor::BinarySensor, link_connected)

  void set_vedirect_id(std::string vedirect_id) { this->vedirect_id_ = vedirect_id; }
  void set_auto_create_text_entities(bool value) { this->auto_create_text_entities_ = value; }
  void set_auto_create_hex_entities(bool value) { this->auto_create_hex_entities_ = value; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  void send_hexframe(const HexTxFrame &hex_frame);
  static void send_hexframe(const std::string &vedirect_id, const std::string &payload);

 protected:
  // component config
  const char *logtag_;
  std::string vedirect_id_;
  bool auto_create_text_entities_{true};
  bool auto_create_hex_entities_{true};

  // component state
  bool connected_{false};
  uint32_t millis_last_rx_{0};

  // frame handler state
  uint8_t checksum_;
  uint8_t buf_[VEDIRECT_BUFFER_LEN];
  uint8_t *buf_end_;
  uint8_t *buf_read_;
  uint8_t *buf_write_;
  uint8_t *buf_name_end_;
  uint8_t *buf_value_end_;
  char *name_[VEDIRECT_RECORDS_COUNT];
  char *value_[VEDIRECT_RECORDS_COUNT];
  uint8_t record_count_;

  enum StateAction {
    Continue,
    Flush,
    Hold,
  };

  typedef StateAction (Manager::*state_func)();
  state_func state_func_;
  state_func state_func_hex_backup_;
  StateAction state_idle_();
  StateAction state_name_();
  StateAction state_value_();
  StateAction state_checksum_();
  StateAction state_hex_();
  StateAction enter_state_hex_();

  inline void on_connected_();
  inline void on_disconnected_();
  inline void on_hexframe_(const HexRxFrame &hexframe);
  inline void on_textframe_();

  // These will provide 'map' access either by text record name (text_entities_)
  // or by HEX register id (hex_entities_). Since some HEX registers are also
  // published in TEXT frames we're also trying to map these to the same entity.
  friend class TFEntity;
  std::unordered_map<const char *, TFEntity *, cstring_hash, cstring_eq> text_entities_;
  friend class HexRegister;
  std::unordered_map<uint16_t, HexRegister *> hex_registers_;

  friend class HexFrameTrigger;
  CallbackManager<void(std::string, bool)> hexframe_callback_;
  void add_on_frame_callback(std::function<void(std::string, bool)> callback) {
    this->hexframe_callback_.add(std::move(callback));
  }

  static std::vector<Manager *> managers_;
};

class HexFrameTrigger : public Trigger<std::string, bool> {
 public:
  explicit HexFrameTrigger(Manager *vedirect) {
    vedirect->add_on_frame_callback([this](std::string payload, bool valid) { this->trigger(payload, valid); });
  }
};

template<typename... Ts> class HexFrameSendAction : public Action<Ts...> {
 public:
  HexFrameSendAction() {}
  TEMPLATABLE_VALUE(std::string, vedirect_id)
  TEMPLATABLE_VALUE(std::string, payload)

  void play(Ts... x) { Manager::send_hexframe(this->vedirect_id_.value(x...), this->payload_.value(x...)); }

 protected:
};

}  // namespace m3_vedirect
}  // namespace esphome
