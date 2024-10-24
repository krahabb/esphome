#include "dallas_component.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m3_dallas {

static const char *const TAG = "m3_dallas.sensor";

static const uint8_t DALLAS_MODEL_DS18S20 = 0x10;
static const uint8_t DALLAS_MODEL_DS1822 = 0x22;
static const uint8_t DALLAS_MODEL_DS18B20 = 0x28;
static const uint8_t DALLAS_MODEL_DS1825 = 0x3B;
static const uint8_t DALLAS_MODEL_DS28EA00 = 0x42;
static const uint8_t DALLAS_COMMAND_START_CONVERSION = 0x44;
static const uint8_t DALLAS_COMMAND_READ_SCRATCH_PAD = 0xBE;
static const uint8_t DALLAS_COMMAND_WRITE_SCRATCH_PAD = 0x4E;

void DallasComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DallasComponent...");

  pin_->setup();

  // clear bus with 480µs high, otherwise initial reset in search_vec() fails
  pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  delayMicroseconds(480);

  one_wire_ = new ESPOneWire(pin_);  // NOLINT(cppcoreguidelines-owning-memory)

  std::vector<uint64_t> raw_sensors;
  raw_sensors = this->one_wire_->search_vec();

  for (auto &address : raw_sensors) {
    auto *address8 = reinterpret_cast<uint8_t *>(&address);
    if (crc8(address8, 7) != address8[7]) {
      ESP_LOGW(TAG, "Dallas device 0x%s has invalid CRC.", format_hex(address).c_str());
      continue;
    }
    if (address8[0] != DALLAS_MODEL_DS18S20 && address8[0] != DALLAS_MODEL_DS1822 &&
        address8[0] != DALLAS_MODEL_DS18B20 && address8[0] != DALLAS_MODEL_DS1825 &&
        address8[0] != DALLAS_MODEL_DS28EA00) {
      ESP_LOGW(TAG, "Unknown device type 0x%02X.", address8[0]);
      continue;
    }
    this->found_sensors_.push_back(address);

    if (this->auto_setup_sensors_) {
      // avoid re-generating  pre-configured sensors
      bool skip = false;
      for (auto *sensor : this->sensors_) {
        if (sensor->get_address() == address) {
          skip = true;
          break;
        }
      }
      if (!skip) {
        auto dallastemperaturesensor = this->create_sensor_by_address(address, this->resolution_);
        char sensor_name[64];
        snprintf(sensor_name, sizeof(sensor_name), this->sensor_name_template_.c_str(), App.get_name().c_str(),
                 format_hex(address).c_str());
        dallastemperaturesensor->set_name(sensor_name);
        dallastemperaturesensor->set_unit_of_measurement(this->unit_of_measurement_.c_str());
        dallastemperaturesensor->set_icon(this->icon_.c_str());
        dallastemperaturesensor->set_accuracy_decimals(this->accuracy_decimals_);
        dallastemperaturesensor->set_force_update(false);
        App.register_sensor(dallastemperaturesensor);
      }
    }
  }

  for (auto *sensor : this->sensors_) {
    if (sensor->get_index().has_value()) {
      if (*sensor->get_index() >= this->found_sensors_.size()) {
        this->status_set_error("Sensor configured by index but not found");
        continue;
      }
      sensor->set_address(this->found_sensors_[*sensor->get_index()]);
    }

    if (!sensor->setup_sensor()) {
      this->status_set_error();
    }
  }
}
void DallasComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DallasComponent:");
  LOG_PIN("  Pin: ", this->pin_);
  LOG_UPDATE_INTERVAL(this);

  if (this->found_sensors_.empty()) {
    ESP_LOGW(TAG, "  Found no sensors!");
  } else {
    ESP_LOGD(TAG, "  Found sensors:");
    for (auto &address : this->found_sensors_) {
      ESP_LOGD(TAG, "    0x%s", format_hex(address).c_str());
    }
  }

  for (auto *sensor : this->sensors_) {
    LOG_SENSOR("  ", "Device", sensor);
    if (sensor->get_index().has_value()) {
      ESP_LOGCONFIG(TAG, "    Index %u", *sensor->get_index());
      if (*sensor->get_index() >= this->found_sensors_.size()) {
        ESP_LOGE(TAG, "Couldn't find sensor by index - not connected. Proceeding without it.");
        continue;
      }
    }
    ESP_LOGCONFIG(TAG, "    Address: %s", sensor->get_address_name().c_str());
    ESP_LOGCONFIG(TAG, "    Resolution: %u", sensor->get_resolution());
  }
}

void DallasComponent::register_sensor(DallasTemperatureSensor *sensor) { this->sensors_.push_back(sensor); }
void DallasComponent::update() {
  this->status_clear_warning();

  bool result;
  {
    InterruptLock lock;
    result = this->one_wire_->reset();
  }
  if (!result) {
    if (!this->found_sensors_.empty()) {
      // Only log error if at the start sensors were found (and thus are disconnected during uptime)
      ESP_LOGE(TAG, "Requesting conversion failed");
      this->status_set_warning();
    }

    for (auto *sensor : this->sensors_) {
      sensor->publish_state(NAN);
    }
    return;
  }

  {
    InterruptLock lock;
    this->one_wire_->skip();
    this->one_wire_->write8(DALLAS_COMMAND_START_CONVERSION);
  }

  for (auto *sensor : this->sensors_) {
    if (sensor->get_address() == 0) {
      ESP_LOGV(TAG, "'%s' - Indexed sensor not found at startup, skipping update", sensor->get_name().c_str());
      sensor->publish_state(NAN);
      continue;
    }

    this->set_timeout(sensor->get_address_name(), sensor->millis_to_wait_for_conversion(), [this, sensor] {
      bool res = sensor->read_scratch_pad();

      if (!res) {
        ESP_LOGW(TAG, "'%s' - Resetting bus for read failed!", sensor->get_name().c_str());
        sensor->publish_state(NAN);
        this->status_set_warning();
        return;
      }
      if (!sensor->check_scratch_pad()) {
        sensor->publish_state(NAN);
        this->status_set_warning();
        return;
      }

      float tempc = sensor->get_temp_c();
      ESP_LOGD(TAG, "'%s': Got Temperature=%.1f°C", sensor->get_name().c_str(), tempc);
      sensor->publish_state(tempc);
    });
  }
}
DallasTemperatureSensor *DallasComponent::create_sensor_by_address(uint64_t address, uint8_t resolution) {
  auto s = new DallasTemperatureSensor(address, resolution, this);
  this->sensors_.push_back(s);
  return s;
}
DallasTemperatureSensor *DallasComponent::create_sensor_by_index(uint8_t index, uint8_t resolution) {
  auto s = this->create_sensor_by_address(0, resolution);
  s->set_index(index);
  return s;
}

DallasTemperatureSensor::DallasTemperatureSensor(uint64_t address, uint8_t resolution, DallasComponent *parent)
    : parent_(parent) {
  this->set_address(address);
  this->set_resolution(resolution);
}

const std::string &DallasTemperatureSensor::get_address_name() {
  if (this->address_name_.empty()) {
    this->address_name_ = std::string("0x") + format_hex(this->address_);
  }
  return this->address_name_;
}

uint16_t DallasTemperatureSensor::millis_to_wait_for_conversion() const {
  switch (this->resolution_) {
    case 9:
      return 94;
    case 10:
      return 188;
    case 11:
      return 375;
    default:
      return 750;
  }
}

bool IRAM_ATTR DallasTemperatureSensor::read_scratch_pad() {
  auto *wire = this->parent_->one_wire_;

  {
    InterruptLock lock;

    if (!wire->reset()) {
      return false;
    }

    wire->select(this->address_);
    wire->write8(DALLAS_COMMAND_READ_SCRATCH_PAD);

    for (unsigned char &i : this->scratch_pad_) {
      i = wire->read8();
    }
  }

  return true;
}
bool DallasTemperatureSensor::setup_sensor() {
  bool r = this->read_scratch_pad();

  if (!r) {
    ESP_LOGE(TAG, "Reading scratchpad failed: reset");
    return false;
  }
  if (!this->check_scratch_pad())
    return false;

  if (this->scratch_pad_[4] == this->resolution_)
    return false;

  if (this->get_address8()[0] == DALLAS_MODEL_DS18S20) {
    // DS18S20 doesn't support resolution.
    ESP_LOGW(TAG, "DS18S20 doesn't support setting resolution.");
    return false;
  }

  switch (this->resolution_) {
    case 12:
      this->scratch_pad_[4] = 0x7F;
      break;
    case 11:
      this->scratch_pad_[4] = 0x5F;
      break;
    case 10:
      this->scratch_pad_[4] = 0x3F;
      break;
    case 9:
    default:
      this->scratch_pad_[4] = 0x1F;
      break;
  }

  auto *wire = this->parent_->one_wire_;
  {
    InterruptLock lock;
    if (wire->reset()) {
      wire->select(this->address_);
      wire->write8(DALLAS_COMMAND_WRITE_SCRATCH_PAD);
      wire->write8(this->scratch_pad_[2]);  // high alarm temp
      wire->write8(this->scratch_pad_[3]);  // low alarm temp
      wire->write8(this->scratch_pad_[4]);  // resolution
      wire->reset();

      // write value to EEPROM
      wire->select(this->address_);
      wire->write8(0x48);
    }
  }

  delay(20);  // allow it to finish operation
  wire->reset();
  return true;
}

bool DallasTemperatureSensor::check_scratch_pad() {
  bool chksum_validity = (crc8(this->scratch_pad_, 8) == this->scratch_pad_[8]);
  bool config_validity = false;

  switch (this->get_address8()[0]) {
    case DALLAS_MODEL_DS18B20:
      config_validity = ((this->scratch_pad_[4] & 0x9F) == 0x1F);
      break;
    default:
      config_validity = ((this->scratch_pad_[4] & 0x10) == 0x10);
  }

#ifdef ESPHOME_LOG_LEVEL_VERY_VERBOSE
  ESP_LOGVV(TAG, "Scratch pad: %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X (%02X)", this->scratch_pad_[0],
            this->scratch_pad_[1], this->scratch_pad_[2], this->scratch_pad_[3], this->scratch_pad_[4],
            this->scratch_pad_[5], this->scratch_pad_[6], this->scratch_pad_[7], this->scratch_pad_[8],
            crc8(this->scratch_pad_, 8));
#endif
  if (!chksum_validity) {
    ESP_LOGW(TAG, "'%s' - Scratch pad checksum invalid!", this->get_name().c_str());
  } else if (!config_validity) {
    ESP_LOGW(TAG, "'%s' - Scratch pad config register invalid!", this->get_name().c_str());
  }
  return chksum_validity && config_validity;
}

float DallasTemperatureSensor::get_temp_c() {
  int16_t temp = (int16_t(this->scratch_pad_[1]) << 11) | (int16_t(this->scratch_pad_[0]) << 3);
  if (this->get_address8()[0] == DALLAS_MODEL_DS18S20) {
    int diff = (this->scratch_pad_[7] - this->scratch_pad_[6]) << 7;
    temp = ((temp & 0xFFF0) << 3) - 16 + (diff / this->scratch_pad_[7]);
  }

  return temp / 128.0f;
}

std::string DallasTemperatureSensor::unique_id() { return "dallas-" + str_upper_case(format_hex(this->address_)); }

}  // namespace m3_dallas
}  // namespace esphome
