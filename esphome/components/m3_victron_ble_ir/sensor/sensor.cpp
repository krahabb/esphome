#include "sensor.h"
#include "esphome/core/log.h"
#include "../protocol.h"

namespace esphome {
namespace m3_victron_ble_ir {

static const char *const TAG = "m3_victron_ble_ir.sensor";

void VictronSensor::setup() {
  switch (this->type_) {
    case VICTRON_SENSOR_TYPE::AUX_VOLTAGE:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::BATTERY_MONITOR:
            if (msg->data.battery_monitor.aux_input_type == VE_REG_BMV_AUX_INPUT::VE_REG_DC_CHANNEL2_VOLTAGE) {
              this->publish_state(0.01f * msg->data.battery_monitor.aux_input.aux_voltage);
            } else {
              ESP_LOGW(TAG, "[%s] Incorrect Aux input configuration.", this->parent_->address_str());
              this->publish_state(NAN);
            }
            break;
          case VBI_RECORD::HEADER::TYPE::DC_ENERGY_METER:
            if (msg->data.dc_energy_meter.aux_input_type == VE_REG_BMV_AUX_INPUT::VE_REG_DC_CHANNEL2_VOLTAGE) {
              this->publish_state(0.01f * msg->data.dc_energy_meter.aux_input.aux_voltage);
            } else {
              ESP_LOGW(TAG, "[%s] Incorrect Aux input configuration.", this->parent_->address_str());
              this->publish_state(NAN);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `aux voltage` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;

    case VICTRON_SENSOR_TYPE::ERROR:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::SMART_LITHIUM:
            this->publish_state((u_int16_t) msg->data.smart_lithium.SmartLithium_error);
            break;
          case VBI_RECORD::HEADER::TYPE::SMART_BATTERY_PROTECT:
            this->publish_state((u_int8_t) msg->data.smart_battery_protect.error_code);
            break;
          case VBI_RECORD::HEADER::TYPE::LYNX_SMART_BMS:
            this->publish_state((u_int8_t) msg->data.lynx_smart_bms.error);
            break;
          case VBI_RECORD::HEADER::TYPE::VE_BUS:
            this->publish_state((u_int8_t) msg->data.ve_bus.ve_bus_error);
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `error` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;

    case VICTRON_SENSOR_TYPE::INPUT_VOLTAGE:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::DCDC_CONVERTER:
            this->publish_state(0.01f * msg->data.dcdc_converter.input_voltage);
            break;
          case VBI_RECORD::HEADER::TYPE::SMART_BATTERY_PROTECT:
            this->publish_state(0.01f * msg->data.smart_battery_protect.input_voltage);
            break;
          case VBI_RECORD::HEADER::TYPE::ORION_XS:
            if (msg->data.orion_xs.input_voltage == 0xFFFF) {
              this->publish_state(0.0f);
            } else {
              this->publish_state(0.01f * msg->data.orion_xs.input_voltage);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `input voltage` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;

    case VICTRON_SENSOR_TYPE::LOAD_CURRENT:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::SOLAR_CHARGER:
            if (msg->data.solar_charger.load_current == 0x1FF) {
              this->publish_state(0);
            } else {
              this->publish_state(-0.1f * msg->data.solar_charger.load_current);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `load current` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;

    case VICTRON_SENSOR_TYPE::MID_VOLTAGE:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::BATTERY_MONITOR:
            if (msg->data.battery_monitor.aux_input_type == VE_REG_BMV_AUX_INPUT::VE_REG_BATTERY_MID_POINT_VOLTAGE) {
              this->publish_state(0.01f * msg->data.battery_monitor.aux_input.mid_voltage);
            } else {
              ESP_LOGW(TAG, "[%s] Incorrect Aux input configuration.", this->parent_->address_str());
              this->publish_state(NAN);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `mid voltage` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;

    case VICTRON_SENSOR_TYPE::OUTPUT_VOLTAGE:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::DCDC_CONVERTER:
            if (msg->data.dcdc_converter.output_voltage == 0x7FFF) {
              this->publish_state(0.0f);
            } else {
              this->publish_state(0.01f * msg->data.dcdc_converter.output_voltage);
            }
            break;
          case VBI_RECORD::HEADER::TYPE::SMART_BATTERY_PROTECT:
            if (msg->data.smart_battery_protect.output_voltage == 0xFFFF) {
              this->publish_state(0.0f);
            } else {
              this->publish_state(0.01f * msg->data.smart_battery_protect.output_voltage);
            }
            break;
          case VBI_RECORD::HEADER::TYPE::ORION_XS:
            if (msg->data.orion_xs.output_voltage == 0xFFFF) {
              this->publish_state(0.0f);
            } else {
              this->publish_state(0.01f * msg->data.orion_xs.output_voltage);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `output voltage` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;

    case VICTRON_SENSOR_TYPE::TEMPERATURE:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::BATTERY_MONITOR:
            if (msg->data.battery_monitor.aux_input_type == VE_REG_BMV_AUX_INPUT::VE_REG_BAT_TEMPERATURE) {
              if (msg->data.battery_monitor.aux_input.temperature == 0xFFFF) {
                this->publish_state(NAN);
              } else {
                this->publish_state(0.01f * msg->data.battery_monitor.aux_input.temperature - 273.15f);
              }
            } else {
              ESP_LOGW(TAG, "[%s] Incorrect Aux input configuration.", this->parent_->address_str());
              this->publish_state(NAN);
            }
            break;
          case VBI_RECORD::HEADER::TYPE::DC_ENERGY_METER:
            if (msg->data.dc_energy_meter.aux_input_type == VE_REG_BMV_AUX_INPUT::VE_REG_BAT_TEMPERATURE) {
              if (msg->data.dc_energy_meter.aux_input.temperature == 0xFFFF) {
                this->publish_state(NAN);
              } else {
                this->publish_state(0.01f * msg->data.dc_energy_meter.aux_input.temperature - 273.15f);
              }
            } else {
              ESP_LOGW(TAG, "[%s] Incorrect Aux input configuration.", this->parent_->address_str());
              this->publish_state(NAN);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `temperature` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;
      // SMART_LITHIUM
    case VICTRON_SENSOR_TYPE::BMS_FLAGS:
    case VICTRON_SENSOR_TYPE::CELL1:
    case VICTRON_SENSOR_TYPE::CELL2:
    case VICTRON_SENSOR_TYPE::CELL3:
    case VICTRON_SENSOR_TYPE::CELL4:
    case VICTRON_SENSOR_TYPE::CELL5:
    case VICTRON_SENSOR_TYPE::CELL6:
    case VICTRON_SENSOR_TYPE::CELL7:
    case VICTRON_SENSOR_TYPE::CELL8:
    case VICTRON_SENSOR_TYPE::BALANCER_STATUS:
      this->parent_->add_on_smart_lithium_message_callback([this](const VICTRON_BLE_RECORD_SMART_LITHIUM *val) {
        switch (this->type_) {
          case VICTRON_SENSOR_TYPE::BMS_FLAGS:
            this->publish_state((u_int32_t) val->bms_flags);
            break;
          case VICTRON_SENSOR_TYPE::CELL1:
            if (val->cell1 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell1 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::CELL2:
            if (val->cell2 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell2 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::CELL3:
            if (val->cell3 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell3 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::CELL4:
            if (val->cell4 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell4 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::CELL5:
            if (val->cell5 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell5 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::CELL6:
            if (val->cell6 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell6 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::CELL7:
            if (val->cell7 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell7 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::CELL8:
            if (val->cell8 == 0x7F) {
              this->publish_state(NAN);
            } else {
              this->publish_state(0.01f * val->cell8 + 2.60f);
            }
            break;
          case VICTRON_SENSOR_TYPE::BALANCER_STATUS:
            this->publish_state((u_int8_t) val->balancer_status);
            break;
          default:
            break;
        }
      });
      break;

      // LYNX_SMART_BMS
    case VICTRON_SENSOR_TYPE::IO_STATUS:
    case VICTRON_SENSOR_TYPE::WARNINGS_ALARMS:
      this->parent_->add_on_lynx_smart_bms_message_callback([this](const VICTRON_BLE_RECORD_LYNX_SMART_BMS *val) {
        switch (this->type_) {
          case VICTRON_SENSOR_TYPE::IO_STATUS:
            this->publish_state((u_int16_t) val->io_status);
            break;
          case VICTRON_SENSOR_TYPE::WARNINGS_ALARMS:
            this->publish_state((u_int32_t) val->warnings_alarms);
            break;
          default:
            break;
        }
      });
      break;

      // VE_BUS
    case VICTRON_SENSOR_TYPE::ALARM:
      this->parent_->add_on_ve_bus_message_callback([this](const VICTRON_BLE_RECORD_VE_BUS *val) {
        switch (this->type_) {
          case VICTRON_SENSOR_TYPE::ALARM:
            this->publish_state((u_int8_t) val->alarm);
            break;
          default:
            break;
        }
      });
      break;

      // DC_ENERGY_METER
    case VICTRON_SENSOR_TYPE::BMV_MONITOR_MODE:
      this->parent_->add_on_dc_energy_meter_message_callback([this](const VICTRON_BLE_RECORD_DC_ENERGY_METER *val) {
        switch (this->type_) {
          case VICTRON_SENSOR_TYPE::BMV_MONITOR_MODE:
            this->publish_state((int16_t) val->bmv_monitor_mode);
            break;
          default:
            break;
        }
      });
      break;

      // ORION_XS
    case VICTRON_SENSOR_TYPE::OUTPUT_CURRENT:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::ORION_XS:
            if (msg->data.orion_xs.output_current == 0xFFFF) {
              this->publish_state(0);
            } else {
              this->publish_state(0.1f * msg->data.orion_xs.output_current);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `output current` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;
    case VICTRON_SENSOR_TYPE::INPUT_CURRENT:
      this->parent_->add_on_message_callback([this](const VBI_RECORD *msg) {
        switch (msg->header.record_type) {
          case VBI_RECORD::HEADER::TYPE::ORION_XS:
            if (msg->data.orion_xs.input_current == 0xFFFF) {
              this->publish_state(0);
            } else {
              this->publish_state(0.1f * msg->data.orion_xs.input_current);
            }
            break;
          default:
            ESP_LOGW(TAG, "[%s] Device has no `input current` field.", this->parent_->address_str());
            this->publish_state(NAN);
            break;
        }
      });
      break;

    default:
      break;
  }
}

const char *VBISensor::UNITS[] = {
    "A", "V", "VA", "W", "Ah", "kWh", "%", "min", "°C",
};
const char *VBISensor::DEVICE_CLASSES[] = {
    "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};
const float VBISensor::DIGITS_TO_SCALE[] = {1.f, .1f, .01f, .001f};

VBISensor::VBISensor(TYPE type) : VBIEntity(type) {
  auto def = this->def;
  this->set_name(def->label);
  this->set_object_id(this->calculate_object_id_());
  switch (def->cls) {
    case CLASS::MEASURE:
      this->set_state_class(sensor::StateClass::STATE_CLASS_MEASUREMENT);
      goto _setup_numeric_sensor;
    case CLASS::MEASURE_TOTAL:
      this->set_state_class(sensor::StateClass::STATE_CLASS_TOTAL);
      goto _setup_numeric_sensor;
    case CLASS::MEASURE_INCREASING:
      this->set_state_class(sensor::StateClass::STATE_CLASS_TOTAL_INCREASING);
      goto _setup_numeric_sensor;
    default:
      return;
  }

_setup_numeric_sensor:
  this->set_unit_of_measurement(UNITS[def->unit]);
  this->set_device_class(DEVICE_CLASSES[def->unit]);
}

void VBISensor::init(const RECORD_DEF *record_def) {
  this->VBIEntity::init(record_def);
  switch (this->def->cls) {
    case CLASS::MEASURE:
    case CLASS::MEASURE_TOTAL:
    case CLASS::MEASURE_INCREASING:
      this->set_accuracy_decimals(record_def->decimal_digits);
      this->scale_ = DIGITS_TO_SCALE[record_def->decimal_digits];
      if (record_def->is_signed) {
        this->signed_offset_ = this->data_mask_ + 1;
        this->nan_value_ = this->data_mask_ >> 1;
        if (this->data_shift_ == 0) {
          if (this->data_mask_ == 0xFF) {
            this->set_parse_func_(parse_signed_t_<u_int8_t>);
            break;
          }
          if (this->data_mask_ == 0xFFFF) {
            this->set_parse_func_(parse_signed_t_<u_int16_t>);
            break;
          }
        }
        this->set_parse_func_(parse_signed_t_<u_int32_t>);
        break;
      } else {  // unsigned
        this->nan_value_ = this->data_mask_;
        switch (this->def->type) {
          case TYPE::BAT_TEMPERATURE:
            this->set_parse_func_(parse_temperature_);
            break;
          default:
            if (this->data_shift_ == 0) {
              if (this->data_mask_ == 0xFF) {
                this->set_parse_func_(parse_unsigned_t_<u_int8_t>);
                break;
              }
              if (this->data_mask_ == 0xFFFF) {
                this->set_parse_func_(parse_unsigned_t_<u_int16_t>);
                break;
              }
            }
            this->set_parse_func_(parse_unsigned_t_<u_int32_t>);
        }
        break;
      }
    case CLASS::ENUM:
    case CLASS::BITMASK:
      if (this->data_shift_ == 0) {
        if (this->data_mask_ == 0xFF) {
          this->set_parse_func_(parse_bitmask_enum_t_<u_int8_t>);
          break;
        }
        if (this->data_mask_ == 0xFFFF) {
          this->set_parse_func_(parse_bitmask_enum_t_<u_int16_t>);
          break;
        }
      }
      this->set_parse_func_(parse_bitmask_enum_t_<u_int32_t>);
      break;
    default:
      this->init_unsupported_();
  }
}

void VBISensor::link_disconnected() {
  if (this->raw_value_ != this->nan_value_) {
    this->raw_value_ = this->nan_value_;
    this->publish_state(NAN);
  }
}

template<typename T> void VBISensor::parse_bitmask_enum_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else
      static_cast<VBISensor *>(entity)->publish_state(value);
  }
}

template<typename T> void VBISensor::parse_signed_t_(VBIEntity *entity, const VBI_RECORD *record) {
  u_int32_t value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    auto sensor = static_cast<VBISensor *>(entity);
    if (value == sensor->nan_value_) {
      sensor->publish_state(NAN);
    } else if (value < sensor->nan_value_) {
      sensor->publish_state(value * sensor->scale_);
    } else {
      sensor->publish_state(((int32_t) value - sensor->signed_offset_) * sensor->scale_);
    }
  }
}

template<typename T> void VBISensor::parse_unsigned_t_(VBIEntity *entity, const VBI_RECORD *record) {
  T value = entity->read_record_t_<T>(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else {
      auto sensor = static_cast<VBISensor *>(entity);
      sensor->publish_state(value * sensor->scale_);
    }
  }
}

void VBISensor::parse_temperature_(VBIEntity *entity, const VBI_RECORD *record) {
  int value = (int) entity->read_record_(record);
  if (value != entity->raw_value_) {
    entity->raw_value_ = value;
    if (value == entity->nan_value_)
      static_cast<VBISensor *>(entity)->publish_state(NAN);
    else {
      static_cast<VBISensor *>(entity)->publish_state(value - 40);
    }
  }
}

}  // namespace m3_victron_ble_ir
}  // namespace esphome