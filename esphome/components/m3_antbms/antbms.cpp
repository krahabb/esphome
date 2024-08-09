#include "antbms.h"

namespace esphome {
namespace m3_antbms {

static const char *const TAG = "m3_antbms";
const byte FRAME_POLL[] = {0xDB, 0xDB, 0x00, 0x00, 0x00, 0x00};

void AntBms::setup() {}
void AntBms::dump_config() {}
void AntBms::update() {
  // executes the standard poll
  this->empty_uart_buffer_();
  this->write_array(FRAME_POLL, sizeof(FRAME_POLL));
  this->set_timeout("read_poll_frame_", FRAME_POLL_READ_TIMEOUT_MS, [this]() { this->read_poll_frame_(); });
}
void AntBms::loop() {
  // TODO: process async SET/GET commands
  PollingComponent::loop();
}

void AntBms::empty_uart_buffer_() {
  byte data;
  while (this->available()) {
    this->read_byte(&data);
  }
}

void AntBms::read_poll_frame_() {
  FramePoll frame;
  if (this->read_array(frame.bytes, sizeof(frame.bytes))) {
    if (!frame.checksum_ok()) {
      ESP_LOGD(TAG, "Invalid checksum");
    }

    if (this->battery_voltage_) {
      float value = frame.BatteryVoltage.to_le() * 0.1;
      if (value != this->battery_voltage_->get_raw_state())
        this->battery_voltage_->publish_state(value);
    }
    if (this->battery_current_) {
      float value = frame.BatteryCurrent.to_le() * 0.1;
      if (value != this->battery_current_->get_raw_state())
        this->battery_current_->publish_state(value);
    }
    if (this->soc_) {
      float value = frame.Soc;
      if (value != this->soc_->get_raw_state())
        this->soc_->publish_state(value);
    }
  }
}

}  // namespace m3_antbms
}  // namespace esphome