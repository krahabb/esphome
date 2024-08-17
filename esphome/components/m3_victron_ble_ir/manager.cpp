#include "manager.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace m3_victron_ble_ir {

static const char *const TAG = "m3_victron_ble_ir";

void VictronBle::dump_config() {
  ESP_LOGCONFIG(TAG, "Victron BLE:");
  ESP_LOGCONFIG(TAG, "  Address: %s", this->address_str());
}

void VictronBle::setup() {
  esp_aes_init(&this->aes_ctx_);
  auto status = esp_aes_setkey(&this->aes_ctx_, this->bindkey_.data(), this->bindkey_.size() * 8);
  if (status != 0) {
    ESP_LOGE(TAG, "[%s] Error during esp_aes_setkey operation (%i).", this->address_str(), status);
    esp_aes_free(&this->aes_ctx_);
    this->mark_failed();
  }
}
/**
 * Parse all incoming BLE payloads to see if it is a Victron BLE advertisement.
 */

bool VictronBle::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_) {
    return false;
  }

  const auto &manu_datas = device.get_manufacturer_datas();
  if (manu_datas.size() != 1) {
    return false;
  }

  const auto &manu_data = manu_datas[0];
  if (manu_data.uuid != esp32_ble_tracker::ESPBTUUID::from_uint16(VICTRON_MANUFACTURER_ID) ||
      manu_data.data.size() <= sizeof(VICTRON_BLE_RECORD) ||
      manu_data.data.size() > (sizeof(VICTRON_BLE_RECORD) + VICTRON_ENCRYPTED_DATA_MAX_SIZE)) {
    return false;
  }

  // Parse the unencrypted data.
  const auto *victron_data = (const VICTRON_BLE_RECORD *) manu_data.data.data();

  if (victron_data->header.record_type != VICTRON_BLE_RECORD::HEADER::TYPE::PRODUCT_ADVERTISEMENT) {
    return false;
  }

  if (victron_data->encryption_key_0 != this->bindkey_[0]) {
    ESP_LOGW(TAG, "[%s] Incorrect Bindkey. Must start with %02X", this->address_str(), this->bindkey_[0]);
    return false;
  }

  // Filter out duplicate messages
  if (victron_data->data_counter == this->last_package_.data_counter) {
    return false;
  }

  const u_int8_t crypted_len = manu_data.data.size() - sizeof(VICTRON_BLE_RECORD);
  ESP_LOGVV(TAG, "[%s] Cryted message: %s", this->address_str(),
            format_hex_pretty(victron_data->data, crypted_len).c_str());

  u_int8_t encrypted_data[VICTRON_ENCRYPTED_DATA_MAX_SIZE] = {0};

  if (crypted_len > sizeof(encrypted_data)) {
    ESP_LOGW(TAG, "[%s] Record is too long %u", this->address_str(), crypted_len);
    return false;
  }

  //
  // decrypt data
  //
  size_t nc_offset = 0;
  u_int8_t nonce_counter[16] = {(u_int8_t) (victron_data->data_counter & 0xff),
                                (u_int8_t) (victron_data->data_counter >> 8), 0};
  u_int8_t stream_block[16] = {0};

  auto status = esp_aes_crypt_ctr(&this->aes_ctx_, crypted_len, &nc_offset, nonce_counter, stream_block,
                                  victron_data->data, encrypted_data);
  if (status != 0) {
    ESP_LOGE(TAG, "[%s] Error during esp_aes_crypt_ctr operation (%i).", this->address_str(), status);
    return false;
  }

  ESP_LOGV(TAG, "[%s] Encrypted message: %s", this->address_str(),
           format_hex_pretty(encrypted_data, crypted_len).c_str());

  this->last_package_.record_type = victron_data->record_type;
  this->last_package_.data_counter = victron_data->data_counter;
  memcpy(this->last_package_.data.raw, encrypted_data, VICTRON_ENCRYPTED_DATA_MAX_SIZE);
  if (this->on_message_callback_.size()) {
    this->defer("VictronBle0", [this]() { this->on_message_callback_.call(&this->last_package_); });
  }

  size_t expected_len = 0;
  switch (victron_data->record_type) {
    case VICTRON_BLE_RECORD::TYPE::SOLAR_CHARGER:
      ESP_LOGD(TAG, "[%s] Recieved SOLAR_CHARGER message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_SOLAR_CHARGER)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_SOLAR_CHARGER);
        goto _error_len;
      }
      if (this->on_solar_charger_message_callback_.size()) {
        this->defer("VictronBle1", [this]() {
          this->on_solar_charger_message_callback_.call(&this->last_package_.data.solar_charger);
        });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::BATTERY_MONITOR:
      ESP_LOGD(TAG, "[%s] Recieved BATTERY_MONITOR message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_BATTERY_MONITOR)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_BATTERY_MONITOR);
        goto _error_len;
      }
      if (this->on_battery_monitor_message_callback_.size()) {
        this->defer("VictronBle2", [this]() {
          this->on_battery_monitor_message_callback_.call(&this->last_package_.data.battery_monitor);
        });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::INVERTER:
      ESP_LOGD(TAG, "[%s] Recieved INVERTER message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_INVERTER)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_INVERTER);
        goto _error_len;
      }
      if (this->on_inverter_message_callback_.size()) {
        this->defer("VictronBle3",
                    [this]() { this->on_inverter_message_callback_.call(&this->last_package_.data.inverter); });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::DCDC_CONVERTER:
      ESP_LOGD(TAG, "[%s] Recieved DCDC_CONVERTER message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_DCDC_CONVERTER)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_DCDC_CONVERTER);
        goto _error_len;
      }
      if (this->on_dcdc_converter_message_callback_.size()) {
        this->defer("VictronBle4", [this]() {
          this->on_dcdc_converter_message_callback_.call(&this->last_package_.data.dcdc_converter);
        });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::SMART_LITHIUM:
      ESP_LOGD(TAG, "[%s] Recieved SMART_LITHIUM message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_SMART_LITHIUM)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_SMART_LITHIUM);
        goto _error_len;
      }
      if (this->on_smart_lithium_message_callback_.size()) {
        this->defer("VictronBle5", [this]() {
          this->on_smart_lithium_message_callback_.call(&this->last_package_.data.smart_lithium);
        });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::INVERTER_RS:
      ESP_LOGD(TAG, "[%s] Recieved INVERTER_RS message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_INVERTER_RS)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_INVERTER_RS);
        goto _error_len;
      }
      if (this->on_inverter_rs_message_callback_.size()) {
        this->defer("VictronBle6",
                    [this]() { this->on_inverter_rs_message_callback_.call(&this->last_package_.data.inverter_rs); });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::SMART_BATTERY_PROTECT:
      ESP_LOGD(TAG, "[%s] Recieved SMART_BATTERY_PROTECT message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_SMART_BATTERY_PROTECT)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_SMART_BATTERY_PROTECT);
        goto _error_len;
      }
      if (this->on_smart_battery_protect_message_callback_.size()) {
        this->defer("VictronBle9", [this]() {
          this->on_smart_battery_protect_message_callback_.call(&this->last_package_.data.smart_battery_protect);
        });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::LYNX_SMART_BMS:
      ESP_LOGD(TAG, "[%s] Recieved LYNX_SMART_BMS message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_LYNX_SMART_BMS)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_LYNX_SMART_BMS);
        goto _error_len;
      }
      if (this->on_lynx_smart_bms_message_callback_.size()) {
        this->defer("VictronBleA", [this]() {
          this->on_lynx_smart_bms_message_callback_.call(&this->last_package_.data.lynx_smart_bms);
        });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::MULTI_RS:
      ESP_LOGD(TAG, "[%s] Recieved MULTI_RS message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_MULTI_RS)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_MULTI_RS);
        goto _error_len;
      }
      if (this->on_multi_rs_message_callback_.size()) {
        this->defer("VictronBleB",
                    [this]() { this->on_multi_rs_message_callback_.call(&this->last_package_.data.multi_rs); });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::VE_BUS:
      ESP_LOGD(TAG, "[%s] Recieved VE_BUS message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_VE_BUS)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_VE_BUS);
        goto _error_len;
      }
      if (this->on_ve_bus_message_callback_.size()) {
        this->defer("VictronBleC",
                    [this]() { this->on_ve_bus_message_callback_.call(&this->last_package_.data.ve_bus); });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::DC_ENERGY_METER:
      ESP_LOGD(TAG, "[%s] Recieved DC_ENERGY_METER message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_DC_ENERGY_METER)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_DC_ENERGY_METER);
        goto _error_len;
      }
      if (this->on_dc_energy_meter_message_callback_.size()) {
        this->defer("VictronBleD", [this]() {
          this->on_dc_energy_meter_message_callback_.call(&this->last_package_.data.dc_energy_meter);
        });
      }
      break;
    case VICTRON_BLE_RECORD::TYPE::ORION_XS:
      ESP_LOGD(TAG, "[%s] Recieved ORION_XS message.", this->address_str());
      if (crypted_len < sizeof(VICTRON_BLE_RECORD_ORION_XS)) {
        expected_len = sizeof(VICTRON_BLE_RECORD_ORION_XS);
        goto _error_len;
      }
      break;
    default:
      break;
  }
  return true;

_error_len:
  ESP_LOGW(TAG, "[%s] Record type %02X message is too short %u, expected %u bytes.", this->address_str(),
           (u_int8_t) victron_data->record_type, crypted_len, expected_len);
  return false;
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome

#endif
