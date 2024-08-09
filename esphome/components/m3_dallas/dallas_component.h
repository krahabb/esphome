#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esp_one_wire.h"

#include <vector>

namespace esphome {
namespace m3_dallas {

class DallasTemperatureSensor;

class DallasComponent : public PollingComponent {
 public:
  void set_pin(InternalGPIOPin *pin) { pin_ = pin; }
  void register_sensor(DallasTemperatureSensor *sensor);

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void update() override;

  /// Get/Set properties for automatically generated sensors.
  void set_auto_setup_sensors(bool auto_setup_sensors) { this->auto_setup_sensors_ = auto_setup_sensors; }
  void set_sensor_name_template(const std::string &sensor_name_template) {
    this->sensor_name_template_ = sensor_name_template;
  }
  void set_resolution(uint8_t resolution) { this->resolution_ = resolution; }
  void set_accuracy_decimals(int8_t accuracy_decimals) { this->accuracy_decimals_ = accuracy_decimals; }
  void set_unit_of_measurement(const std::string &unit_of_measurement) {
    this->unit_of_measurement_ = unit_of_measurement;
  }
  void set_icon(const std::string &icon) { this->icon_ = icon; }

 protected:
  friend DallasTemperatureSensor;

  InternalGPIOPin *pin_;
  ESPOneWire *one_wire_;
  std::vector<DallasTemperatureSensor *> sensors_;
  std::vector<uint64_t> found_sensors_;

  bool auto_setup_sensors_;
  std::string sensor_name_template_;
  uint8_t resolution_;
  std::string unit_of_measurement_;
  std::string icon_;
  int8_t accuracy_decimals_;

  DallasTemperatureSensor *create_sensor_by_address(uint64_t address, uint8_t resolution);
  DallasTemperatureSensor *create_sensor_by_index(uint8_t index, uint8_t resolution);
};

/// Internal class that helps us create multiple sensors for one Dallas hub.
class DallasTemperatureSensor : public sensor::Sensor {
 public:
  DallasTemperatureSensor(uint64_t address, uint8_t resolution, DallasComponent *parent);

  void set_parent(DallasComponent *parent) { parent_ = parent; }

  uint64_t get_address() { return this->address_; }
  void set_address(uint64_t address) { this->address_ = address; }

  uint8_t *get_address8() { return reinterpret_cast<uint8_t *>(&this->address_); }
  /// Helper to create (and cache) the name for this sensor. For example "0xfe0000031f1eaf29".
  const std::string &get_address_name();

  uint8_t get_resolution() const { return this->resolution_; }
  void set_resolution(uint8_t resolution) { this->resolution_ = resolution; }

  /// Get the index of this sensor. (0 if using address.)
  optional<uint8_t> get_index() const { return this->index_; }
  /// Set the index of this sensor. If using index, address will be set after setup.
  void set_index(uint8_t index) { this->index_ = index; }

  /// Get the number of milliseconds we have to wait for the conversion phase.
  uint16_t millis_to_wait_for_conversion() const;

  bool setup_sensor();
  bool read_scratch_pad();

  bool check_scratch_pad();

  float get_temp_c();

  std::string unique_id() override;

 protected:
  DallasComponent *parent_;
  uint64_t address_;
  optional<uint8_t> index_;

  uint8_t resolution_;
  std::string address_name_;
  uint8_t scratch_pad_[9] = {
      0,
  };
};

}  // namespace m3_dallas
}  // namespace esphome
