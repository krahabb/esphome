#include "FujitsuClimate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m3_fujitsuac {

static const char *const TAG = "fujitsuac";
const std::string FUJISEND_TIMEOUT_NAME("fujisend");

static const climate::ClimateMode FUJITSU_TO_ESPHOME_ACMODE[] = {
    climate::ClimateMode::CLIMATE_MODE_AUTO,      // UNKNOWN = 0,
    climate::ClimateMode::CLIMATE_MODE_FAN_ONLY,  // FAN = 1,
    climate::ClimateMode::CLIMATE_MODE_DRY,       // DRY = 2,
    climate::ClimateMode::CLIMATE_MODE_COOL,      // COOL = 3,
    climate::ClimateMode::CLIMATE_MODE_HEAT,      // HEAT = 4,
    climate::ClimateMode::CLIMATE_MODE_AUTO       // AUTO = 5,
};
#define FUJITSU_TO_ESPHOME_ACMODE_COUNT (sizeof(FUJITSU_TO_ESPHOME_ACMODE) / sizeof(FUJITSU_TO_ESPHOME_ACMODE[0]))

static const uint8_t ESPHOME_TO_FUJITSU_ACMODE[] = {
    FUJITSUAC_ACMODE_UNKNOWN,  // CLIMATE_MODE_OFF = 0,
    FUJITSUAC_ACMODE_AUTO,     // CLIMATE_MODE_HEAT_COOL = 1,
    FUJITSUAC_ACMODE_COOL,     // CLIMATE_MODE_COOL = 2,
    FUJITSUAC_ACMODE_HEAT,     // CLIMATE_MODE_HEAT = 3,
    FUJITSUAC_ACMODE_FAN,      // CLIMATE_MODE_FAN_ONLY = 4,
    FUJITSUAC_ACMODE_DRY,      // CLIMATE_MODE_DRY = 5,
    FUJITSUAC_ACMODE_AUTO      // CLIMATE_MODE_AUTO = 6
};
#define ESPHOME_TO_FUJITSU_ACMODE_COUNT (sizeof(ESPHOME_TO_FUJITSU_ACMODE) / sizeof(ESPHOME_TO_FUJITSU_ACMODE[0]))

static const climate::ClimateFanMode FUJITSU_TO_ESPHOME_FANMODE[] = {
    climate::ClimateFanMode::CLIMATE_FAN_AUTO,    // FAN_AUTO = 0,
    climate::ClimateFanMode::CLIMATE_FAN_FOCUS,   // FAN_QUIET = 1,
    climate::ClimateFanMode::CLIMATE_FAN_LOW,     // FAN_LOW = 2,
    climate::ClimateFanMode::CLIMATE_FAN_MEDIUM,  // FAN_MEDIUM = 3,
    climate::ClimateFanMode::CLIMATE_FAN_HIGH,    // FAN_HIGH = 4
};
#define FUJITSU_TO_ESPHOME_FANMODE_COUNT (sizeof(FUJITSU_TO_ESPHOME_FANMODE) / sizeof(FUJITSU_TO_ESPHOME_FANMODE[0]))

static const uint8_t ESPHOME_TO_FUJITSU_FANMODE[] = {
    FUJITSUAC_FANMODE_AUTO,    // CLIMATE_FAN_ON = 0,
    FUJITSUAC_FANMODE_AUTO,    // CLIMATE_FAN_OFF = 1,
    FUJITSUAC_FANMODE_AUTO,    // CLIMATE_FAN_AUTO = 2,
    FUJITSUAC_FANMODE_LOW,     // CLIMATE_FAN_LOW = 3,
    FUJITSUAC_FANMODE_MEDIUM,  // CLIMATE_FAN_MEDIUM = 4,
    FUJITSUAC_FANMODE_HIGH,    // CLIMATE_FAN_HIGH = 5,
    FUJITSUAC_FANMODE_MEDIUM,  // CLIMATE_FAN_MIDDLE = 6,
    FUJITSUAC_FANMODE_QUIET,   // CLIMATE_FAN_FOCUS = 7,
    FUJITSUAC_FANMODE_QUIET    // CLIMATE_FAN_DIFFUSE = 8,
};
#define ESPHOME_TO_FUJITSU_FANMODE_COUNT (sizeof(ESPHOME_TO_FUJITSU_FANMODE) / sizeof(ESPHOME_TO_FUJITSU_FANMODE[0]))

void FujitsuClimate::setup() {
  /*use GPIO5 to enable/disable TX line adapter
  this is needed because GPIO15(our TX) doesn't want a pull-up on start
  and our BJT polarization circuit would 'induce' a pull-up
  should we tie the BJT to a fixed 3.3V
  By using GPIO5 to enable the BJT buffer we're avoiding a fixed (high)
  level on the TX pin (GPIO15) at boot which would then halt the boot process
  */
  this->highfrequencyloop_.start();
}

void FujitsuClimate::loop() {
  FujiFrame frame;
  if (read_frame(frame)) {
    this->lastframemillis_ = frame.millis;
    if (this->address_ == frame.addr_src) {
      // skip our tx frames
    } else if (this->address_ == frame.addr_dst) {
      this->lastboundframemillis_ = frame.millis;
      switch (frame.type) {
        case FUJITSUAC_FRAMETYPE_STATUS:
          update_state(frame);
          if (frame.bit_controllerpresent) {
            this->state_ = Bound;
            merge_state();
            send_statusframe();
          } else {
            switch (this->address_) {
              case FUJITSUAC_ADDR_PRIMARYCONTROLLER:
                send_loginframe();
                break;
              default:
                /*
                    according to https://github.com/unreality/FujiHeatPump
                    the secondary doesnt do any login procedure and just chains
                    it's frames after the PRIMARY
                */
                this->state_ = Bound;
                merge_state();
                send_statusframe();
                break;
            }
          }
          break;
        case FUJITSUAC_FRAMETYPE_LOGIN:
          this->state_ = Bound;
          send_statusframe();
          break;
        case FUJITSUAC_FRAMETYPE_ERROR:
          break;
        default:  // FUJITSUAC_FRAMETYPE_UNKNOWN
          break;
      }
    } else {  // analyze frames for other devices too
      switch (frame.type) {
        case FUJITSUAC_FRAMETYPE_STATUS:
          if (frame.bit_error) {
            /*
                my ac keeps sending an ac_error status every 30 secs or so in a
                frame with this structure:  {00,80,40,80,00,00,00,00}
                addr_src == 0 addr_dst == 0
            */
            break;
          }
          if (!frame.bit_write) {
            /*
                we passively monitor any frame to look for status.
                This is useful if our controller is not bound or for any other reason
                and provides a consistent status reflection of the unit to HA
            */
            update_state(frame);
            /*if (frame.addr_src == FUJITSUAC_ADDR_PRIMARYCONTROLLER)
                send_statusframe();*/
          }
          break;
        case FUJITSUAC_FRAMETYPE_LOGIN:
          break;
        case FUJITSUAC_FRAMETYPE_ERROR:
          break;
        default:  // FUJITSUAC_FRAMETYPE_UNKNOWN
          break;
      }
    }

    if (this->state_ == Disconnected)
      this->state_ = Connected;
  }

  // manage disconnection timeouts
  switch (this->state_) {
    case Bound:
      if ((millis() - this->lastboundframemillis_) < 2000)
        break;
      this->state_ = Connected;
    case Connected:
      if ((millis() - this->lastframemillis_) < 1000)
        break;
      this->state_ = Disconnected;
    default:
      break;
  }
}

void FujitsuClimate::dump_config() {
  char buffer[256];
  ESP_LOGCONFIG(TAG, "Fujitsu AC:");
  ESP_LOGCONFIG(TAG, "  address: %d", this->address_);
  LOG_PIN("  TX Pin: ", this->txenable_pin_);
  /*for (FujiFrame frame : this->frames_log_) {
    ESP_LOGCONFIG(TAG, "%s", frame.dump_payload(buffer));
    ESP_LOGCONFIG(TAG, "%s", frame.dump_decoded(buffer));
  }*/
}

void FujitsuClimate::control(const climate::ClimateCall &call) {
  this->call_mode_ = call.get_mode();
  this->call_target_temperature_ = call.get_target_temperature();

  /*if (call.get_mode().has_value()) {
      auto fujitsu_mode = ESPHOME_TO_FUJITSU_MODE[call.get_mode().value()];
      switch (fujitsu_mode)
      {
      case FujiMode::UNKNOWN:
          this->heatpump_.setOnOff(false);
          break;
      default:
          this->heatpump_.setOnOff(true);
          this->heatpump_.setMode((byte)fujitsu_mode);
      }
      ESP_LOGD(TAG, "Fuji setting mode %d", (int)fujitsu_mode);
  }

  if (call.get_fan_mode().has_value()) {
      auto fujitsu_fanmode = ESPHOME_TO_FUJITSU_FANMODE[call.get_fan_mode().value()];
      this->heatpump_.setFanMode((byte)fujitsu_fanmode);
      ESP_LOGD(TAG, "Fuji setting fan mode %d", (int)fujitsu_fanmode);
  }

  if (call.get_target_temperature().has_value()) {
      auto target_temperature = call.get_target_temperature().value();
      this->heatpump_.setTemp(target_temperature);
      ESP_LOGD(TAG, "Fuji setting temperature %f", target_temperature);
  }

  if (call.get_preset().has_value()) {
      auto callPreset = call.get_preset().value();
      this->sharedState.economyMode = static_cast<byte>(
          callPreset == climate::ClimatePreset::CLIMATE_PRESET_ECO ? 1
                                                                      : 0);
      updated = true;
      ESP_LOGD(TAG, "Fuji setting preset %d", callPreset);
  }*/
}

climate::ClimateTraits FujitsuClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supports_current_temperature(true);
  traits.set_supported_modes({
      climate::CLIMATE_MODE_AUTO,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_FAN_ONLY,
      climate::CLIMATE_MODE_DRY,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_OFF,
  });

  traits.set_visual_temperature_step(1);
  traits.set_visual_min_temperature(16);
  traits.set_visual_max_temperature(30);

  traits.set_supported_fan_modes({climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                                  climate::CLIMATE_FAN_HIGH, climate::CLIMATE_FAN_FOCUS});
  /*traits.set_supported_presets({
      climate::CLIMATE_PRESET_ECO,
      climate::CLIMATE_PRESET_NONE,
  });*/

  return traits;
}

bool FujitsuClimate::read_frame(FujiFrame &frame) {
  if (uart::UARTDevice::available()) {
    /*
        since the esphome uart::UARTDevice::read_array
        is 'make-it-or-break-it' we'll read 1 char at a time
        in order to read up to whatever is available (reading timout
        looks like 100 msec in esphome core).
        This will allow us to realign on frame boundaries should we loose
        (or get spurious) chars on the bus
    */
    for (frame.datalen = 0; frame.datalen < 8; ++frame.datalen) {
      if (!uart::UARTDevice::read_byte(frame.data + frame.datalen)) {
        ESP_LOGD(TAG, "Timeout reading frame on the serial bus");
        break;
      }
    }
    frame.millis = millis();
    for (int i = frame.datalen; i > 0;) {
      frame.data[--i] ^= 0xFF;
    }
    char buffer[256];
    ESP_LOGI(TAG, "RECV: %s", frame.dump_payload(buffer));
    ESP_LOGI(TAG, "RECV: %s", frame.dump_decoded(buffer));

    // collect the first transactions on the bus
    // for protocol state machine inspection
    if (this->frames_log_.size() < 30) {
      this->frames_log_.push_back(frame);
    } else {
      const FujiFrame &logframe = this->frames_log_[this->frames_log_dump_index_];
      ESP_LOGD(TAG, "RECVLOG(%i): %s", this->frames_log_dump_index_, logframe.dump_payload(buffer));
      ESP_LOGD(TAG, "RECVLOG(%i): %s", this->frames_log_dump_index_, logframe.dump_decoded(buffer));
      if (++this->frames_log_dump_index_ >= this->frames_log_.size())
        this->frames_log_dump_index_ = 0;
    }
    return frame.datalen == 8;
  }
  return false;
}

void FujitsuClimate::update_state(FujiFrame &frame) {
  this->statusframe_ = frame;

  bool update = false;
  if (this->target_temperature != frame.temp_target) {
    this->target_temperature = frame.temp_target;
    update = true;
  }
  if (frame.addr_src >= 32) {
    // extract room temperature from controller unit(s)
    if (this->current_temperature != frame.temp_room) {
      this->current_temperature = frame.temp_room;
      update = true;
    }
  }
  if (frame.enabled) {
    uint8_t fujitsu_mode = frame.acmode;
    if ((fujitsu_mode > 0) && (fujitsu_mode < FUJITSU_TO_ESPHOME_ACMODE_COUNT)) {
      auto esphome_mode = FUJITSU_TO_ESPHOME_ACMODE[fujitsu_mode];
      if (this->mode != esphome_mode) {
        this->mode = esphome_mode;
        update = true;
      }
      climate::ClimateAction esphome_action = climate::ClimateAction::CLIMATE_ACTION_IDLE;
      switch (esphome_mode) {
        case climate::ClimateMode::CLIMATE_MODE_FAN_ONLY:
          esphome_action = climate::ClimateAction::CLIMATE_ACTION_FAN;
          break;
        case climate::ClimateMode::CLIMATE_MODE_DRY:
          esphome_action = climate::ClimateAction::CLIMATE_ACTION_DRYING;
          break;
        case climate::ClimateMode::CLIMATE_MODE_COOL:
          if (this->target_temperature < this->current_temperature)
            esphome_action = climate::ClimateAction::CLIMATE_ACTION_COOLING;
          break;
        case climate::ClimateMode::CLIMATE_MODE_HEAT:
          if (this->target_temperature > this->current_temperature)
            esphome_action = climate::ClimateAction::CLIMATE_ACTION_HEATING;
          break;
        default:
          if (this->target_temperature > this->current_temperature)
            esphome_action = climate::ClimateAction::CLIMATE_ACTION_HEATING;
          else if (this->target_temperature < this->current_temperature)
            esphome_action = climate::ClimateAction::CLIMATE_ACTION_COOLING;
          break;
      }
      if (this->action != esphome_action) {
        this->action = esphome_action;
        update = true;
      }
    }
  } else {
    if (this->mode != climate::ClimateMode::CLIMATE_MODE_OFF) {
      this->mode = climate::ClimateMode::CLIMATE_MODE_OFF;
      update = true;
    }
    if (this->action != climate::ClimateAction::CLIMATE_ACTION_OFF) {
      this->action = climate::ClimateAction::CLIMATE_ACTION_OFF;
      update = true;
    }
  }

  uint8_t fujitsu_fanmode = frame.fanmode;
  if (fujitsu_fanmode < FUJITSU_TO_ESPHOME_FANMODE_COUNT) {
    auto esphome_fanmode = FUJITSU_TO_ESPHOME_FANMODE[fujitsu_fanmode];
    if (this->fan_mode != esphome_fanmode) {
      this->fan_mode = esphome_fanmode;
      update = true;
    }
  }

  if (update)
    this->publish_state();
}

void FujitsuClimate::merge_state() {
  if (this->call_mode_.has_value()) {
    this->statusframe_.bit_write = 1;
    auto fujitsu_acmode = ESPHOME_TO_FUJITSU_ACMODE[this->call_mode_.value()];
    switch (fujitsu_acmode) {
      case FUJITSUAC_ACMODE_UNKNOWN:
        this->statusframe_.enabled = 0;
        break;
      default:
        this->statusframe_.enabled = 1;
        this->statusframe_.acmode = fujitsu_acmode;
    }
    this->call_mode_ = nullopt;
    ESP_LOGD(TAG, "Fuji setting mode %d", (int) fujitsu_acmode);
  }

  if (this->call_target_temperature_.has_value()) {
    uint8_t targettemp = this->call_target_temperature_.value();
    if (targettemp != this->statusframe_.temp_target) {
      this->statusframe_.bit_write = 1;
      this->statusframe_.temp_target = targettemp;
    }
    this->call_target_temperature_ = nullopt;
    ESP_LOGD(TAG, "Fuji setting target temp %d", (int) targettemp);
  }
}

void FujitsuClimate::send_frame(FujiFrame &frame) {
  frame.addr_src = this->address_;
  frame.addr_dst = FUJITSUAC_ADDR_UNIT;
  frame.bit_unknown = 1;
  frame.millis = millis();

  char buffer[256];
  ESP_LOGD(TAG, "SEND: %s", frame.dump_payload(buffer));
  ESP_LOGD(TAG, "SEND: %s", frame.dump_decoded(buffer));

  for (int i = 0; i < FUJITSUAC_FRAMESIZE; ++i)
    this->sendbuf_[i] = frame.data[i] ^ 0xFF;

  unsigned long waitfor = millis() - this->lastframemillis_;
  if (waitfor < 50)
    this->set_timeout(FUJISEND_TIMEOUT_NAME, 50 - waitfor, [this]() { this->internal_send_frame(); });
  else
    ESP_LOGE(TAG, "Timeout exceeded while synchronizing transmission");
}

void FujitsuClimate::send_loginframe() {
  FujiFrame loginframe;
  loginframe.type = FUJITSUAC_FRAMETYPE_LOGIN;
  send_frame(loginframe);
}

void FujitsuClimate::send_statusframe() {
  // status frame comes from the master or other controllers...
  // set proper fields in our response and go...
  this->statusframe_.temp_room = this->current_temperature;
  this->statusframe_.updatemagic = (this->address_ == FUJITSUAC_ADDR_PRIMARYCONTROLLER) ? 0 : 2;
  this->statusframe_.bit_controllerpresent = 1;
  this->statusframe_.data[7] = 0;
  send_frame(this->statusframe_);
}

void FujitsuClimate::internal_send_frame() {
  if (this->txenable_pin_)
    this->txenable_pin_->digital_write(1);

  uart::UARTDevice::write_array(this->sendbuf_, FUJITSUAC_FRAMESIZE);
  uart::UARTDevice::flush();

  if (this->txenable_pin_)
    this->txenable_pin_->digital_write(0);

  /*uint8_t recvbuf[FUJITSUAC_FRAMESIZE];
  if (!uart::UARTDevice::read_array(recvbuf, FUJITSUAC_FRAMESIZE)) {
    ESP_LOGD(TAG, "Timeout reading back transmitted frame on the serial bus");
    return;
  }
  if (0 != memcmp(recvbuf, this->sendbuf_, FUJITSUAC_FRAMESIZE)) {
    ESP_LOGD(TAG, "TX collision while sending frame");
    return;
  }*/
}

}  // namespace m3_fujitsuac
}  // namespace esphome
