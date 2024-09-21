#include "manager.h"
#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "sensor/sensor.h"
#include "text_sensor/text_sensor.h"

namespace esphome {
namespace m3_victron_ble_ir {

static const char *const TAG = "m3_victron_ble_ir";

void Manager::dump_config() {
  ESP_LOGCONFIG(TAG, "Victron BLE:");
  ESP_LOGCONFIG(TAG, "  Address: %s", this->address_str());
}

void Manager::setup() {
  if (this->auto_create_entities_ && (this->auto_create_type_ < VBI_RECORD::HEADER::TYPE::_COUNT)) {
    this->auto_create_(this->auto_create_type_);
  }

#ifdef DEBUG_VBIENTITY
  this->time_loop_ = millis();
#endif
#ifdef USE_ESP32
  esp_aes_init(&this->aes_ctx_);
  auto status = esp_aes_setkey(&this->aes_ctx_, this->bindkey_.data(), this->bindkey_.size() * 8);
  if (status != 0) {
    ESP_LOGE(TAG, "[%s] Error during esp_aes_setkey operation (%i).", this->address_str(), status);
    esp_aes_free(&this->aes_ctx_);
    this->mark_failed();
  }
#endif
}

void Manager::loop() {
  uint32_t t = millis();
#ifdef DEBUG_VBIENTITY

  if ((t - this->time_loop_) > 2000) {
    VBI_RECORD test_record;

    if (this->auto_create_type_ < VBI_RECORD::HEADER::TYPE::_COUNT)
      test_record.header.record_type = this->auto_create_type_;
    else
      test_record.header.record_type = VBI_RECORD::HEADER::MULTI_RS;
    switch (test_record.header.record_type) {
      case VBI_RECORD::HEADER::MULTI_RS:
        switch (t % 3) {
          case 0:
            test_record.data.multi_rs.active_ac_out_power = -1;
            test_record.data.multi_rs.active_ac_in_power = -2;
            test_record.data.multi_rs.yield_today = 100;
            test_record.data.multi_rs.pv_power = 1;
            test_record.data.multi_rs.battery_current = -32768;
            test_record.data.multi_rs.battery_voltage = 0;
            test_record.data.multi_rs.device_state = VE_REG_DEVICE_STATE::INVERTING;
            test_record.data.multi_rs.charger_error = VE_REG_CHR_ERROR_CODE::BMS;
            test_record.data.multi_rs.active_ac_in = VE_REG_AC_IN_ACTIVE::AC_IN_1;
            break;
          case 1:
            test_record.data.multi_rs.active_ac_out_power = 32766;
            test_record.data.multi_rs.active_ac_in_power = 10;
            test_record.data.multi_rs.yield_today = 100;
            test_record.data.multi_rs.pv_power = 65534;
            test_record.data.multi_rs.battery_current = 32766;
            test_record.data.multi_rs.battery_voltage = 16382;
            test_record.data.multi_rs.device_state = VE_REG_DEVICE_STATE::BULK;
            test_record.data.multi_rs.charger_error = VE_REG_CHR_ERROR_CODE::CURRENT_SENSOR;
            test_record.data.multi_rs.active_ac_in = VE_REG_AC_IN_ACTIVE::AC_IN_2;
            break;
          default:
            test_record.data.multi_rs.active_ac_out_power = 0x7FFF;
            test_record.data.multi_rs.active_ac_in_power = 0x7FFF;
            test_record.data.multi_rs.yield_today = 0xFFFF;
            test_record.data.multi_rs.pv_power = 0xFFFF;
            test_record.data.multi_rs.battery_current = 32767;
            test_record.data.multi_rs.battery_voltage = 16383;
            test_record.data.multi_rs.device_state = VE_REG_DEVICE_STATE::NOT_AVAILABLE;
            test_record.data.multi_rs.charger_error = VE_REG_CHR_ERROR_CODE::NOT_AVAILABLE;
            test_record.data.multi_rs.active_ac_in = VE_REG_AC_IN_ACTIVE::UNKNOWN;
        }
        break;
      case VBI_RECORD::HEADER::VE_BUS:
        if (t % 2) {
          test_record.data.ve_bus.device_state = VE_REG_DEVICE_STATE::INVERTING;
        } else {
          test_record.data.ve_bus.device_state = VE_REG_DEVICE_STATE::BULK;
        }
        test_record.data.ve_bus.alarm = (VE_REG_ALARM_NOTIFICATION) (t % 4);
        break;
      default:
        break;
    }

    if (this->auto_create_entities_) {
      this->auto_create_(test_record.header.record_type);
    }

    for (auto entity : this->entities_) {
      entity->parse(&test_record);
    }

    this->time_loop_ = t;
  }
#endif
}
/**
 * Parse all incoming BLE payloads to see if it is a Victron BLE advertisement.
 */
#ifdef USE_ESP32
bool Manager::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_) {
    return false;
  }

  const auto &manu_datas = device.get_manufacturer_datas();
  if (manu_datas.size() != 1) {
    return false;
  }

  const auto &manu_data = manu_datas[0];
  const auto record_size = manu_data.data.size();
  if (manu_data.uuid != esp32_ble_tracker::ESPBTUUID::from_uint16(VICTRON_MANUFACTURER_ID) ||
      (record_size <= sizeof(VBI_RECORD::HEADER)) || (record_size > sizeof(VBI_RECORD))) {
    return false;
  }

  // Parse the unencrypted data.
  const auto *victron_data = (const VBI_RECORD *) manu_data.data.data();

  if (victron_data->header.manufacturer_record_type !=
      VBI_RECORD::HEADER::MANUFACTURER_RECORD_TYPE::PRODUCT_ADVERTISEMENT) {
    return false;
  }

  if (victron_data->header.encryption_key_0 != this->bindkey_[0]) {
    ESP_LOGW(TAG, "[%s] Incorrect Bindkey. Must start with %02X", this->address_str(), this->bindkey_[0]);
    return false;
  }

  // Filter out duplicate messages
  if (victron_data->header.data_counter == this->record_.header.data_counter) {
    return false;
  }

  const u_int8_t crypted_len = record_size - sizeof(VBI_RECORD::HEADER);
  ESP_LOGVV(TAG, "[%s] Cryted message: %s", this->address_str(),
            format_hex_pretty(victron_data->data, crypted_len).c_str());

  //
  // decrypt data
  //
  size_t nc_offset = 0;
  u_int8_t nonce_counter[16] = {(u_int8_t) (victron_data->header.data_counter & 0xff),
                                (u_int8_t) (victron_data->header.data_counter >> 8), 0};
  u_int8_t stream_block[16] = {0};

  auto status = esp_aes_crypt_ctr(&this->aes_ctx_, crypted_len, &nc_offset, nonce_counter, stream_block,
                                  victron_data->data.raw, this->record_.data.raw);
  if (status != 0) {
    ESP_LOGE(TAG, "[%s] Error during esp_aes_crypt_ctr operation (%i).", this->address_str(), status);
    return false;
  }

  ESP_LOGV(TAG, "[%s] Encrypted message: %s", this->address_str(),
           format_hex_pretty(encrypted_data, crypted_len).c_str());

  memcpy(&this->record_.header, &victron_data->header, sizeof(VBI_RECORD::HEADER));

  if (this->on_message_callback_.size()) {
    this->defer("VictronBle0", [this]() { this->on_message_callback_.call(&this->record_); });
  }

  if (this->auto_create_entities_) {
    this->auto_create_(this->record_.header.record_type);
  }

  for (auto entity : this->entities_) {
    entity->parse(&this->record_);
  }

  if (this->link_connected_) {
    this->link_connected_->publish_state(true);
    this->set_timeout("link_connected", this->link_connected_timeout_, [this]() { this->timeout_link_connected_(); });
  }

  return true;
}
#endif

void Manager::auto_create_(VBI_RECORD::HEADER::TYPE record_type) {
  // disable calling again
  this->auto_create_entities_ = false;

  api::APIServer *api_server = api::global_api_server;
  if (api_server && (api_server->get_component_state() == COMPONENT_STATE_CONSTRUCTION))
    // api_server eventually not initialized yet
    api_server = nullptr;

  // for every defined entity 'TYPE'
  for (auto &def : VBIEntity::DEFS) {
    // check if it was not already defined (yaml config)
    bool existing = false;
    for (auto entity : this->entities_) {
      // When checking for existing entities we exclude (eventual) BinarySensors
      // since those can be mapped to individual BITMASKs or ENUMs and
      // we want (always?) instead create a plain Text/Sensor for the 'raw' data field
      if ((entity->def == &def) && !entity->is_binary_sensor()) {
        existing = true;
        break;
      }
    }
    if (existing)
      continue;
    // scan the RECORD_DEFS looking for a matching 'record_type'
    for (const auto *record_def = def.record_types; record_def->record_type < VBI_RECORD::HEADER::_COUNT;
         ++record_def) {
      if (record_def->record_type == record_type) {
        // entity 'TYPE' has a definition for incoming 'record_type'
        switch (def.cls) {
          case VBIEntity::CLASS::BITMASK:
          case VBIEntity::CLASS::ENUM: {
            auto text_sensor = new VBITextSensor(def.type);
            text_sensor->init(record_def);
            App.register_text_sensor(text_sensor);
            if (api_server)
              text_sensor->add_on_state_callback([text_sensor](const std::string &state) {
                api::global_api_server->on_text_sensor_update(text_sensor, state);
              });
            this->register_entity(text_sensor);
          } break;
          default: {
            auto sensor = new VBISensor(def.type);
            sensor->init(record_def);
            App.register_sensor(sensor);
            if (api_server)
              sensor->add_on_state_callback(
                  [sensor](float state) { api::global_api_server->on_sensor_update(sensor, state); });
            this->register_entity(sensor);
          }
        }
      }
    }
  }
}

void Manager::timeout_link_connected_() {
  for (auto entity : this->entities_) {
    entity->link_disconnected();
  }
  this->link_connected_->publish_state(false);
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome
