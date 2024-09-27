#include "antbms.h"
#include "esphome/core/application.h"

#ifdef USE_ESP_IDF
#include "esp_system.h"
#endif

namespace esphome {
namespace m3_antbms {

static const char *const TAG = "m3_antbms";
static const char *const ENTITY_ARRAY_NAME_FMT = "%s_%02u";
static const char *const ENTITY_ARRAY_OBJECT_ID_FMT = ENTITY_ARRAY_NAME_FMT;
const byte FRAME_POLL[] = {0xDB, 0xDB, 0x00, 0x00, 0x00, 0x00};

#define ARRAY_SIZE(_array) (sizeof(_array) / sizeof(_array[0]))

SensorConfig::SensorConfig(AntBms *_antbms, const char *_name, EntityBase *_entity, int _offset)
    : entity(_entity), offset(_offset) {
  std::string object_id = _entity->get_object_id();
  if (object_id.empty()) {
    char object_id_buf[256];
    if (_antbms->get_object_id_prefix()) {
      sprintf(object_id_buf, "%s_%s", _antbms->get_object_id_prefix(), _name);
    } else {
      sprintf(object_id_buf, "%s", _name);
    }
    _entity->set_object_id(object_id_buf);
  }
}

const char *const TextSensorConfig::CHARGE_MOS_MAP[] = {
    /* 0x00 */ "Off",
    /* 0x01 */ "On",
    /* 0x02 */ "Over charge protection",
    /* 0x03 */ "Over current protection",
    /* 0x04 */ "Battery full",
    /* 0x05 */ "Battery over voltage",
    /* 0x06 */ "Battery over temperature",
    /* 0x07 */ "MOSFET over temperature",
    /* 0x08 */ "Abnormal current",
    /* 0x09 */ "Balanced line dropped string",
    /* 0x0A */ "Motherboard over temperature",
    /* 0x0B */ "11",
    /* 0x0C */ "12",
    /* 0x0D */ "Discharge MOSFET abnormality",
    /* 0x0E */ "14",
    /* 0x0F */ "Manually turned off",
};
const int TextSensorConfig::CHARGE_MOS_MAP_SIZE = ARRAY_SIZE(TextSensorConfig::CHARGE_MOS_MAP);
const char *const TextSensorConfig::DISCHARGE_MOS_MAP[] = {
    /* 0x00 */ "Off",
    /* 0x01 */ "On",
    /* 0x02 */ "Over discharge protection",
    /* 0x03 */ "Over current protection",
    /* 0x04 */ "4",
    /* 0x05 */ "Battery under voltage",
    /* 0x06 */ "Battery over temperature",
    /* 0x07 */ "MOSFET over temperature",
    /* 0x08 */ "Abnormal current",
    /* 0x09 */ "Balanced line dropped string",
    /* 0x0A */ "Motherboard over temperature",
    /* 0x0B */ "Charge MOSFET on",
    /* 0x0C */ "Short circuit protection",
    /* 0x0D */ "Discharge MOSFET abnormality",
    /* 0x0E */ "Start exception",
    /* 0x0F */ "Manually turned off",
};
const int TextSensorConfig::DISCHARGE_MOS_MAP_SIZE = ARRAY_SIZE(TextSensorConfig::DISCHARGE_MOS_MAP);
const char *const TextSensorConfig::BALANCE_MAP[] = {
    /* 0x00 */ "Off",
    /* 0x01 */ "Exceeds the limit equilibrium",
    /* 0x02 */ "Charge differential pressure balance",
    /* 0x03 */ "Balancer over temperature",
    /* 0x04 */ "Automatic equalization",
    /* 0x05 */ "5",
    /* 0x06 */ "6",
    /* 0x07 */ "7",
    /* 0x08 */ "8",
    /* 0x09 */ "9",
    /* 0x0A */ "Motherboard over temperature",
};
const int TextSensorConfig::BALANCE_MAP_SIZE = ARRAY_SIZE(TextSensorConfig::BALANCE_MAP);

const char *CellSensorConfig::OBJECT_ID = "cell_voltage";
void AntBms::set_cell_voltage(sensor::Sensor *cell_voltage, int count) {
  // yaml configuration will just setup a 'default' single entity
  char name[256];
  std::string original_name = cell_voltage->get_name();
  if (original_name.empty())
    original_name = "Cell voltage";
  sprintf(name, ENTITY_ARRAY_NAME_FMT, original_name.c_str(), 1);
  cell_voltage->set_name(strdup(name));
  char object_id[256];
  std::string original_object_id = cell_voltage->get_object_id();
  if (original_object_id.empty()) {
    if (this->get_object_id_prefix()) {
      sprintf(object_id, "%s_%s", this->get_object_id_prefix(), CellSensorConfig::OBJECT_ID);
      original_object_id = object_id;
    } else {
      original_object_id = CellSensorConfig::OBJECT_ID;
    }
  }
  sprintf(object_id, ENTITY_ARRAY_OBJECT_ID_FMT, original_object_id.c_str(), 1);
  cell_voltage->set_object_id(strdup(object_id));
  // insert the first one (mainly configured by yaml)
  this->sensors_.push_back(new CellSensorConfig(this, cell_voltage, 6));

  for (int i = 2; i <= count; ++i) {
    // now clone the configured entity and fix naming/object_id
    auto sensor = new sensor::Sensor();
    sprintf(name, ENTITY_ARRAY_NAME_FMT, original_name.c_str(), i);
    sensor->set_name(strdup(name));
    sprintf(object_id, ENTITY_ARRAY_OBJECT_ID_FMT, original_object_id.c_str(), i);
    sensor->set_object_id(strdup(object_id));
    sensor->set_entity_category(cell_voltage->get_entity_category());
    sensor->set_device_class("voltage");
    sensor->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
    sensor->set_unit_of_measurement("V");
    sensor->set_accuracy_decimals(cell_voltage->get_accuracy_decimals());
    App.register_sensor(sensor);
    this->sensors_.push_back(new CellSensorConfig(this, sensor, 4 + 2 * i));
  }
}

const char *const TEMPERATURE = "temperature";
void AntBms::set_temperature(sensor::Sensor *_sensor, int count) {
  // yaml configuration will just setup a 'default' single entity
  char name[256];  // overflow ?...optimistic approach
  std::string original_name = _sensor->get_name();
  if (original_name.empty())
    original_name = "Temperature";
  sprintf(name, ENTITY_ARRAY_NAME_FMT, original_name.c_str(), 1);
  _sensor->set_name(strdup(name));
  char object_id[256];
  std::string original_object_id = _sensor->get_object_id();
  if (original_object_id.empty()) {
    if (this->get_object_id_prefix()) {
      sprintf(object_id, "%s_%s", this->get_object_id_prefix(), TEMPERATURE);
      original_object_id = object_id;
    } else {
      original_object_id = TEMPERATURE;
    }
  }
  sprintf(object_id, ENTITY_ARRAY_OBJECT_ID_FMT, original_object_id.c_str(), 1);
  _sensor->set_object_id(strdup(object_id));
  // insert the first one (mainly configured by yaml)
  this->sensors_.push_back(new FloatSensorConfig(this, TEMPERATURE, _sensor, 91, be_i16_to_float, 1.f));

  for (int i = 2; i <= count; ++i) {
    // now clone the configured entity and fix naming/object_id
    auto sensor = new sensor::Sensor();
    sprintf(name, ENTITY_ARRAY_NAME_FMT, original_name.c_str(), i);
    sensor->set_name(strdup(name));
    sprintf(object_id, ENTITY_ARRAY_OBJECT_ID_FMT, original_object_id.c_str(), i);
    sensor->set_object_id(strdup(object_id));
    sensor->set_entity_category(_sensor->get_entity_category());
    sensor->set_device_class("temperature");
    sensor->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
    sensor->set_unit_of_measurement("°C");
    sensor->set_accuracy_decimals(0);
    App.register_sensor(sensor);
    this->sensors_.push_back(new FloatSensorConfig(this, TEMPERATURE, sensor, 89 + 2 * i, be_i16_to_float, 1.f));
  }
}

void AntBms::setup() {
  if (this->memory_free_) {
    this->set_interval("free_heap_check_", FREE_HEAP_CHECK_TIMEOUT_MS, [this]() { this->free_heap_check_(); });
  }
  if (this->battery_energy_in_ || this->battery_energy_out_) {
    // if any of the 2 defined then ensure the other is too.
    if (!this->battery_energy_in_) {
      this->battery_energy_in_ = new sensor::Sensor();
      this->battery_energy_in_->set_internal(true);
    } else if (!this->battery_energy_out_) {
      this->battery_energy_out_ = new sensor::Sensor();
      this->battery_energy_out_->set_internal(true);
    }
  }
}
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
  if (!this->available()) {
    // UART likely disconnected..
    this->battery_energy_last_time = 0;  // resets energy accumulation until reconnected
    if (this->link_connected_) {
      this->link_connected_->publish_state(false);
    }
    return;
  }

  if (this->read_array(frame.bytes, sizeof(frame.bytes))) {
    if (frame.checksum_ok()) {
      if (this->battery_energy_in_) {
        // both will be available (see ::setup())
        float battery_power = frame.battery_power.to_le();
        uint32_t time = millis();
        if (this->battery_energy_last_time) {
          uint32_t dt = time - this->battery_energy_last_time;
          // 'de' should be divided by 7200000.f to get Wh but we avoid accumulating
          // division rounding errors. The value(s) will be scaled on publishing.
          float de = (this->battery_energy_last_power + battery_power) * dt;
          sensor::Sensor battery_energy_sensor;
          if (de >= 0) {
            // round down so we only send 1 Wh updates
            float value = (int) ((this->battery_energy_out_value += de) / 7200000.f);
            if (value != this->battery_energy_out_->get_raw_state()) {
              this->battery_energy_out_->publish_state(value);
            }
          } else {
            float value = (int) ((this->battery_energy_in_value -= de) / 7200000.f);
            if (value != this->battery_energy_in_->get_raw_state()) {
              this->battery_energy_in_->publish_state(value);
            }
          }
        }
        this->battery_energy_last_time = time;
        this->battery_energy_last_power = battery_power;
      }

      for (auto sensor_config : this->sensors_) {
        sensor_config->parse(frame);
      }

      if (this->link_connected_) {
        this->link_connected_->publish_state(true);
      }

    } else {
      ESP_LOGD(TAG, "Invalid checksum");
    }
  }
}

void AntBms::free_heap_check_() {
#ifdef USE_ESP_IDF
  float free_heap_kb = esp_get_free_heap_size() / 1024;
  if (free_heap_kb != this->memory_free_->get_raw_state()) {
    this->memory_free_->publish_state(free_heap_kb);
  }
#endif
}

}  // namespace m3_antbms
}  // namespace esphome