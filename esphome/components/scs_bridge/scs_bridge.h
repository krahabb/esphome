#pragma once


#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/core/automation.h"
#include <string>
#include <vector>
#include "scs_device.h"

namespace esphome {
namespace scs_bridge {

#define SCS_ADR_SCSBRIDGE 0x00 //fix this right now..used as our src addr
#define SCS_ADR_BROADCAST_STATUS 0xB8 //targets all devices
#define SCS_ADR_BROADCAST_ROOM 0xB3 //targets room==src_address devices
#define SCS_ADR_BROADCAST_QUERY 0xB1
#define SCS_ACK 0xA5
#define SCS_CMD_SET 0x12
#define SCS_CMD_GET 0x15
#define SCS_VAL_SWITCH_ON 0x00
#define SCS_VAL_SWITCH_OFF 0x01
#define SCS_VAL_COVER_UP 0x08
#define SCS_VAL_COVER_DOWN 0x09
#define SCS_VAL_COVER_STOP 0x0A

class SCSCover;
class SCSSwitch;

class SCSBridge : public Component {
 public:
  static const char *const TAG;

  const std::string device_name_template;

  SCSBridge();
  SCSBridge(uint8_t rx_pin, uint8_t tx_pin, std::string device_name_template);

  void setup() override;
  void loop() override;
  void dump_config() override;

  void add_on_frame_callback(std::function<void(std::string)> callback) {
    this->frame_callback_.add(std::move(callback));
  }

  static SCSBridge *instance() { return instance_; }

  static void send(std::vector<uint8_t> payload, uint8_t repeat, bool acknowledge);
  static void send(std::string payload, uint8_t repeat, bool acknowledge);
  static void send(uint8_t dst_address, uint8_t src_address, uint8_t command, uint8_t value, bool acknowledge);

  static void register_cover(SCSCover *_cover);
  static void register_switch(SCSSwitch *_switch);

 protected:

  static SCSBridge *instance_;

  static std::vector<SCSCover *> covers_;
  SCSCover *getcover_(uint8_t address);

  static std::vector<SCSSwitch *> switches_;
  SCSSwitch *getswitch_(uint8_t address);

  CallbackManager<void(std::string)> frame_callback_;
};

class SCSBridgeFrameTrigger : public Trigger<std::string> {
 public:
  explicit SCSBridgeFrameTrigger(SCSBridge *parent) {
    parent->add_on_frame_callback([this](std::string payload) { this->trigger(payload); });
  }
};


template<typename... Ts> class SCSBridgeSendAction : public Action<Ts...> {
 public:
  SCSBridgeSendAction() {}
  TEMPLATABLE_VALUE(std::string, payload)
  TEMPLATABLE_VALUE(uint8_t, repeat)
  TEMPLATABLE_VALUE(bool, acknowledge)

  void play(Ts... x) {
    SCSBridge::send(this->payload_.value(x...), this->repeat_.value(x...), this->acknowledge_.value(x...));
  }

 protected:

};

}  // namespace scs_bridge
}  // namespace esphome
