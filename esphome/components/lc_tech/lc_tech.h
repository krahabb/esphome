#pragma once


#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"

//#include "esphome.h"
//using namespace esphome;

/*
 The protocol according to the docs:
 Open relay  1:A0 01 01 A2
 Close relay 1:A0 01 00 A1
 Open relay  2:A0 02 01 A3
 Close relay 2:A0 02 00 A2
 Open relay  3:A0 03 01 A4
 Close relay 3:A0 03 00 A3
 Open relay  4:A0 04 01 A5
 Close relay 4:A0 04 00 A4
 - First  byte, always A0 start of the packet packet
 - Second byte, relay number 1-4
 - Third  byte, state 0/1
 - Forth  byte, the sum of three previous bytes
  Alternative firmware also add RGB float outputs and button states
*/

/*
  define an interval before which we're not sending any new
  payload through the serial line since the slave controller is
  a bit tricky (not sure this is needed tho)
*/
#define LCTECH_SERIAL_IDLE_MILLIS 200
/*
  The polling period: used to keep pushing actual state to the slave controller
*/
#define LCTECH_UPDATE_INTERVAL 2000


namespace esphome {
namespace lc_tech {

class LCTechRelay : public PollingComponent, public switch_::Switch {

 public:

  LCTechRelay(uart::UARTComponent *uart, uint8_t channel);

  void setup() override;
  void update() override;

  void write_state(bool state);

 protected:

  uart::UARTComponent * const uart_;
  const uint8_t channel_;
  uint8_t state_{0x00};
  bool restore_state_{true};

  void send_packet();
};

}//namespace lc_tech
}//namespace esphome
