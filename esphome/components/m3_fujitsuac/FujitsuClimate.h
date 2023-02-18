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
        Disconnected,//no activity on the bus
        Connected,//syncrhonized and passively listening
        Bound//the controller can talk to the master unit
    };

    void set_address(uint8_t address) {
        this->address_ = address;
    }
    void set_txenable_pin(GPIOPin* pin) {
        this->txenable_pin_ = pin;
    }

    void setup() override;
    void loop() override;
    void dump_config() override;

    void control(const climate::ClimateCall &call) override;
    climate::ClimateTraits traits() override;

 protected:
    //configuration
    uint8_t         address_={};
    GPIOPin*        txenable_pin_={};

    //state
    State           state_={Disconnected};
    unsigned long   lastframemillis_={};
    unsigned long   lastboundframemillis_={};
    FujiFrame       statusframe_;
    uint8_t         sendbuf_[FUJITSUAC_FRAMESIZE];

    //pending write status
    optional<climate::ClimateMode>      call_mode_;
    optional<float>                     call_target_temperature_;
    optional<climate::ClimateFanMode>   call_fan_mode_;
    optional<climate::ClimateSwingMode> call_swing_mode_;

    //tell esphome to run this at max speed
    HighFrequencyLoopRequester  highfrequencyloop_;

    std::vector<FujiFrame> frames_log_;

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
