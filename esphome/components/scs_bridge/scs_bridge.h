#pragma once


#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/core/automation.h"
#include <string>

namespace esphome {
namespace scs_bridge {


class SCSBridgeComponent : public Component {
 public:
  SCSBridgeComponent();
  SCSBridgeComponent(uint8_t rx_pin, uint8_t tx_pin);
  SCSBridgeComponent(GPIOPin* rx_pin, GPIOPin* tx_pin);


  void setup() override;
  void loop() override;
  void dump_config() override;

  void add_on_frame_callback(std::function<void(std::string)> callback) {
    this->frame_callback_.add(std::move(callback));
  }

  static void send(std::string payload, uint32_t repeat_count);

 protected:
  CallbackManager<void(std::string)> frame_callback_;
};

class SCSBridgeFrameTrigger : public Trigger<std::string> {
 public:
  explicit SCSBridgeFrameTrigger(SCSBridgeComponent *parent) {
    parent->add_on_frame_callback([this](std::string payload) { this->trigger(payload); });
  }
};


template<typename... Ts> class SCSBridgeSendAction : public Action<Ts...> {
 public:
  SCSBridgeSendAction() {}
  TEMPLATABLE_VALUE(std::string, payload)
  TEMPLATABLE_VALUE(uint32_t, repeat_count)

  void play(Ts... x) {
    SCSBridgeComponent::send(this->payload_.value(x...), this->repeat_count_.value(x...));
  }

 protected:

};

}  // namespace scs_bridge
}  // namespace esphome
