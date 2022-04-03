#include "lc_tech.h"
#include "esphome/core/log.h"

static const char *const TAG = "LCTechRelay";

namespace esphome {
namespace lc_tech {

LCTechRelay::LCTechRelay(uart::UARTComponent *uart, uint8_t channel)
  : PollingComponent(LCTECH_UPDATE_INTERVAL), uart_(uart), channel_(channel) {
}

void LCTechRelay::setup() {

  if (this->restore_state_) {
    auto restored = this->get_initial_state();
    if (restored.has_value()) {
      ESP_LOGD(TAG, "  Restored state %s", ONOFF(*restored));
      if (*restored) {
        this->turn_on();
      } else {
        this->turn_off();
      }
      return;
    }
  }
  //default: ensure consistent boot
  //since we cannot read actual relay state
  this->write_state(false);
}

void LCTechRelay::update() {
  //continuously send command since the nuvoton is deaf
  this->send_packet();
}

void LCTechRelay::write_state(bool state) {
  this->state_ = state ? 0x01 : 0x00;
  this->send_packet();
  this->publish_state(state);
}

void LCTechRelay::send_packet()
{
  static int32_t _last_send_millis = 0;

  uint8_t msg[4];
  msg[0] = 0xA0;
  msg[3] = 0xA0 + (msg[1] = this->channel_) + (msg[2] = this->state_);

  int32_t millis_to_wait = _last_send_millis + LCTECH_SERIAL_IDLE_MILLIS - (int32_t)millis();
  if (millis_to_wait > LCTECH_UPDATE_INTERVAL) {
    //arithmetic might be subtle when millis() overflow
    //at any rate it's no use delaying a call beyond our update interval
    return;
  }

  if (millis_to_wait > 0) {
      ESP_LOGD(TAG, "send_packet: delayed = %u millis", millis_to_wait);
      this->set_timeout("send_packet", millis_to_wait, [this]() { this->send_packet(); });
      return;
    }
  _last_send_millis = (int32_t)millis();
  this->uart_->write_array(msg, sizeof(msg));
  this->uart_->flush();
  ESP_LOGD(TAG, "send_packet: channel = %u state = %s", (int)this->channel_, ONOFF(this->state_));
}

}//namespace lc_tech
}//namespace esphome
