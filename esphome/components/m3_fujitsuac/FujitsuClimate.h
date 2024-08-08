#pragma once
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/climate/climate.h"
#include "fujidefs.h"

namespace esphome {
namespace m3_fujitsuac {

class FujitsuClimate : public climate::Climate, public uart::UARTDevice, public Component {
 public:
  enum State {
    Disconnected,  // no activity on the bus
    Connected,     // syncrhonized and passively listening
    Bound          // the controller can talk to the master unit
  };

  void set_address(uint8_t address) { this->address_ = address; }
  void set_txenable_pin(GPIOPin *pin) { this->txenable_pin_ = pin; }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  float get_loop_priority() const override { return 50.0f; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

 protected:
  // configuration
  uint8_t address_{0};
  GPIOPin *txenable_pin_{nullptr};

  // state
  State state_{Disconnected};
  unsigned long lastframemillis_{0};
  unsigned long lastboundframemillis_{0};
  FujiFrame statusframe_{};
  uint8_t sendbuf_[FUJITSUAC_FRAMESIZE];

  // pending write status
  optional<climate::ClimateMode> call_mode_{nullopt};
  optional<float> call_target_temperature_{nullopt};
  optional<climate::ClimateFanMode> call_fan_mode_{nullopt};
  optional<climate::ClimateSwingMode> call_swing_mode_{nullopt};

  // tell esphome to run this at max speed
  HighFrequencyLoopRequester highfrequencyloop_{};

  std::vector<FujiFrame> frames_log_{};
  int frames_log_dump_index_{0};

  bool read_frame(FujiFrame &frame);
  void update_state(FujiFrame &frame);
  void merge_state();

  void send_frame(FujiFrame &frame);
  void send_loginframe();
  void send_statusframe();

  void internal_send_frame();
};

}  // namespace m3_fujitsuac
}  // namespace esphome
