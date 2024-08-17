#pragma once

#ifdef USE_ESP32
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include <aes/esp_aes.h>

#include <array>

#include "protocol.h"

namespace esphome {
namespace m3_victron_ble_ir {

class VictronBle : public esp32_ble_tracker::ESPBTDeviceListener, public Component {
 public:
  void dump_config() override;

  void setup() override;

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;

  void set_address(uint64_t address) {
    this->address_ = address;
    sprintf(this->address_str_, "%02X:%02X:%02X:%02X:%02X:%02X", (uint8_t) (address >> 40) & 0xff,
            (uint8_t) (address >> 32) & 0xff, (uint8_t) (address >> 24) & 0xff, (uint8_t) (address >> 16) & 0xff,
            (uint8_t) (address >> 8) & 0xff, (uint8_t) (address >> 0) & 0xff);
  }

  const char *address_str() const { return this->address_str_; }

  void set_bindkey(std::array<uint8_t, 16> key) { this->bindkey_ = key; }

  void add_on_battery_monitor_message_callback(
      std::function<void(const VICTRON_BLE_RECORD_BATTERY_MONITOR *)> callback) {
    this->on_battery_monitor_message_callback_.add(std::move(callback));
  }
  void add_on_solar_charger_message_callback(std::function<void(const VICTRON_BLE_RECORD_SOLAR_CHARGER *)> callback) {
    this->on_solar_charger_message_callback_.add(std::move(callback));
  }
  void add_on_inverter_message_callback(std::function<void(const VICTRON_BLE_RECORD_INVERTER *)> callback) {
    this->on_inverter_message_callback_.add(std::move(callback));
  }
  void add_on_dcdc_converter_message_callback(std::function<void(const VICTRON_BLE_RECORD_DCDC_CONVERTER *)> callback) {
    this->on_dcdc_converter_message_callback_.add(std::move(callback));
  }
  void add_on_smart_lithium_message_callback(std::function<void(const VICTRON_BLE_RECORD_SMART_LITHIUM *)> callback) {
    this->on_smart_lithium_message_callback_.add(std::move(callback));
  }
  void add_on_inverter_rs_message_callback(std::function<void(const VICTRON_BLE_RECORD_INVERTER_RS *)> callback) {
    this->on_inverter_rs_message_callback_.add(std::move(callback));
  }
  void add_on_smart_battery_protect_message_callback(
      std::function<void(const VICTRON_BLE_RECORD_SMART_BATTERY_PROTECT *)> callback) {
    this->on_smart_battery_protect_message_callback_.add(std::move(callback));
  }
  void add_on_lynx_smart_bms_message_callback(std::function<void(const VICTRON_BLE_RECORD_LYNX_SMART_BMS *)> callback) {
    this->on_lynx_smart_bms_message_callback_.add(std::move(callback));
  }
  void add_on_multi_rs_message_callback(std::function<void(const VICTRON_BLE_RECORD_MULTI_RS *)> callback) {
    this->on_multi_rs_message_callback_.add(std::move(callback));
  }
  void add_on_ve_bus_message_callback(std::function<void(const VICTRON_BLE_RECORD_VE_BUS *)> callback) {
    this->on_ve_bus_message_callback_.add(std::move(callback));
  }
  void add_on_dc_energy_meter_message_callback(
      std::function<void(const VICTRON_BLE_RECORD_DC_ENERGY_METER *)> callback) {
    this->on_dc_energy_meter_message_callback_.add(std::move(callback));
  }
  void add_on_orion_xs_message_callback(std::function<void(const VICTRON_BLE_RECORD_ORION_XS *)> callback) {
    this->on_orion_xs_message_callback_.add(std::move(callback));
  }
  void add_on_message_callback(std::function<void(const VictronBleData *)> callback) {
    this->on_message_callback_.add(std::move(callback));
  }

 protected:
  uint64_t address_{};
  char address_str_[18] = {};

  std::array<uint8_t, 16> bindkey_;

  esp_aes_context aes_ctx_;

  VictronBleData last_package_{};

#define VICTRON_MESSAGE_STORAGE_CB(name, type) CallbackManager<void(const type *)> on_##name##_message_callback_{};

  CallbackManager<void(const VictronBleData *)> on_message_callback_{};
  VICTRON_MESSAGE_STORAGE_CB(battery_monitor, VICTRON_BLE_RECORD_BATTERY_MONITOR)
  VICTRON_MESSAGE_STORAGE_CB(solar_charger, VICTRON_BLE_RECORD_SOLAR_CHARGER)
  VICTRON_MESSAGE_STORAGE_CB(inverter, VICTRON_BLE_RECORD_INVERTER)
  VICTRON_MESSAGE_STORAGE_CB(dcdc_converter, VICTRON_BLE_RECORD_DCDC_CONVERTER)
  VICTRON_MESSAGE_STORAGE_CB(smart_lithium, VICTRON_BLE_RECORD_SMART_LITHIUM)
  VICTRON_MESSAGE_STORAGE_CB(inverter_rs, VICTRON_BLE_RECORD_INVERTER_RS)
  VICTRON_MESSAGE_STORAGE_CB(smart_battery_protect, VICTRON_BLE_RECORD_SMART_BATTERY_PROTECT)
  VICTRON_MESSAGE_STORAGE_CB(lynx_smart_bms, VICTRON_BLE_RECORD_LYNX_SMART_BMS)
  VICTRON_MESSAGE_STORAGE_CB(multi_rs, VICTRON_BLE_RECORD_MULTI_RS)
  VICTRON_MESSAGE_STORAGE_CB(ve_bus, VICTRON_BLE_RECORD_VE_BUS)
  VICTRON_MESSAGE_STORAGE_CB(dc_energy_meter, VICTRON_BLE_RECORD_DC_ENERGY_METER)
  VICTRON_MESSAGE_STORAGE_CB(orion_xs, VICTRON_BLE_RECORD_ORION_XS)

#undef VICTRON_MESSAGE_STORAGE_CB
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome

#endif
