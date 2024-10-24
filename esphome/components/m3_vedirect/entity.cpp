#include "entity.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "manager.h"
#include "binary_sensor/binary_sensor.h"
#include "select/select.h"
#include "sensor/sensor.h"
#include "text_sensor/text_sensor.h"

namespace esphome {
namespace m3_vedirect {

static const char *const TAG = "m3_vedirect.entity";

const char *VEDirectEntity::UNIT_TO_DEVICE_CLASS[] = {
    "current", "voltage", "apparent_power", "power", nullptr, "energy", "battery", "duration", "temperature",
};
const sensor::StateClass VEDirectEntity::UNIT_TO_STATE_CLASS[] = {
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_TOTAL,       sensor::StateClass::STATE_CLASS_TOTAL_INCREASING,
    sensor::StateClass::STATE_CLASS_MEASUREMENT, sensor::StateClass::STATE_CLASS_MEASUREMENT,
    sensor::StateClass::STATE_CLASS_MEASUREMENT,
};

#define DEF_TFBINARYSENSOR(name, disabled) \
  { name, VEDirectEntity::CLASS::BOOLEAN, disabled, UNIT::NONE, DIGITS::D_0 }
#define DEF_TFSENSOR(name, disabled, unit, digits) \
  { name, VEDirectEntity::CLASS::NUMERIC, disabled, unit, digits }
#define DEF_TFTEXTSENSOR(name, disabled) \
  { name, VEDirectEntity::CLASS::ENUM, disabled, UNIT::NONE, DIGITS::D_0 }

const VEDirectEntity::text_def_map_t VEDirectEntity::TEXT_DEFS{
    {"AC_OUT_I", DEF_TFSENSOR("AC output current", false, UNIT::A, DIGITS::D_1)},
    {"AC_OUT_S", DEF_TFSENSOR("AC output apparent power", false, UNIT::VA, DIGITS::D_0)},
    {"AC_OUT_V", DEF_TFSENSOR("AC output voltage", false, UNIT::V, DIGITS::D_2)},
    {"H19", DEF_TFSENSOR("Yield total", false, UNIT::kWh, DIGITS::D_2)},
    {"H20", DEF_TFSENSOR("Yield today", false, UNIT::kWh, DIGITS::D_2)},
    {"H21", DEF_TFSENSOR("Maximum power today", false, UNIT::W, DIGITS::D_0)},
    {"H22", DEF_TFSENSOR("Yield yesterday", true, UNIT::kWh, DIGITS::D_2)},
    {"H23", DEF_TFSENSOR("Maximum power yesterday", true, UNIT::W, DIGITS::D_0)},
    {"I", DEF_TFSENSOR("Battery current", false, UNIT::A, DIGITS::D_3)},
    {"IL", DEF_TFSENSOR("Load current", false, UNIT::A, DIGITS::D_3)},
    {"PPV", DEF_TFSENSOR("PV power", false, UNIT::W, DIGITS::D_0)},
    {"V", DEF_TFSENSOR("Battery voltage", false, UNIT::V, DIGITS::D_3)},
    {"VPV", DEF_TFSENSOR("PV voltage", false, UNIT::V, DIGITS::D_3)},

    {"AR", DEF_TFTEXTSENSOR("Alarm reason", false)},
    {"CS", DEF_TFTEXTSENSOR("State of operation", false)},
    {"ERR", DEF_TFTEXTSENSOR("Error code", false)},
    {"FW", DEF_TFTEXTSENSOR("Firmware version (FW)", true)},
    {"FWE", DEF_TFTEXTSENSOR("Firmware version (FWE)", true)},
    {"MODE", DEF_TFTEXTSENSOR("Device mode", false)},
    {"MPPT", DEF_TFTEXTSENSOR("Tracker operation mode", false)},
    {"OR", DEF_TFTEXTSENSOR("Off reason", false)},
    {"PID", DEF_TFTEXTSENSOR("Product Id", true)},
    {"Relay", DEF_TFTEXTSENSOR("Relay state", false)},
    {"SER#", DEF_TFTEXTSENSOR("Serial number", true)},
    {"WARN", DEF_TFTEXTSENSOR("Warning reason", false)},

    {"Alarm", DEF_TFBINARYSENSOR("Alarm", false)},
    {"LOAD", DEF_TFBINARYSENSOR("Output state", false)},

};

VEDirectEntity *VEDirectEntity::build(Manager *manager, const char *label) {
  VEDirectEntity *entity;
  auto text_def_it = TEXT_DEFS.find(label);
  if (text_def_it == TEXT_DEFS.end()) {
    // ENTITIES_DEF lacks the definition for this parameter so
    // we return a plain TextSensor entity.
    // We allocate a copy since the label param is 'volatile'
    label = strdup(label);
    entity = VEDirectEntity::dynamic_build_entity_<TextSensor>(manager, label, label);
  } else {
    label = text_def_it->first;
    auto &text_def = text_def_it->second;
    switch (text_def.cls) {
      case CLASS::NUMERIC:
        // pass our 'static' copy of the label (param is volatile)
        entity = VEDirectEntity::dynamic_build_entity_<Sensor>(manager, text_def.description, label);
        break;
      case CLASS::BOOLEAN:
        entity = VEDirectEntity::dynamic_build_entity_<BinarySensor>(manager, text_def.description, label);
        break;
      default:
        entity = VEDirectEntity::dynamic_build_entity_<TextSensor>(manager, text_def.description, label);
    }
    entity->init_text_def_(&text_def);
  }
  manager->text_entities_.emplace(label, entity);
  entity->dynamic_register();
  return entity;
}

VEDirectEntity *VEDirectEntity::build(Manager *manager, register_id_t register_id) {
  VEDirectEntity *entity;
  auto reg_def = REG_DEF::find(register_id);
  if (reg_def) {
    switch (reg_def->cls) {
      case CLASS::NUMERIC:
        if (reg_def->access == REG_DEF::ACCESS::READ_ONLY) {
          entity = VEDirectEntity::dynamic_build_entity_<Sensor>(manager, reg_def->label, reg_def->label);
        } else {
          // TODO: build a number entity
          entity = VEDirectEntity::dynamic_build_entity_<Sensor>(manager, reg_def->label, reg_def->label);
        }
        break;
      case CLASS::BOOLEAN:
        if (reg_def->access == REG_DEF::ACCESS::READ_ONLY) {
          entity = VEDirectEntity::dynamic_build_entity_<BinarySensor>(manager, reg_def->label, reg_def->label);
        } else {
          // TODO: build a switch entity
          entity = VEDirectEntity::dynamic_build_entity_<BinarySensor>(manager, reg_def->label, reg_def->label);
        }
        break;
      case CLASS::ENUM:
        if (reg_def->access == REG_DEF::ACCESS::READ_ONLY) {
          entity = VEDirectEntity::dynamic_build_entity_<TextSensor>(manager, reg_def->label, reg_def->label);
        } else {
          // TODO: build a select entity
          entity = VEDirectEntity::dynamic_build_entity_<TextSensor>(manager, reg_def->label, reg_def->label);
        }
        break;
      case CLASS::BITMASK:
        // TODO: build individual binary_sensors/toggles ?
        entity = VEDirectEntity::dynamic_build_entity_<TextSensor>(manager, reg_def->label, reg_def->label);
        break;
      default:
        entity = VEDirectEntity::dynamic_build_entity_<TextSensor>(manager, reg_def->label, reg_def->label);
    }
  } else {
    // else build a raw text sensor
    char *object_id = new char[7];
    sprintf(object_id, "0x%04X", (int) register_id);
    char *name = new char[16];
    sprintf(name, "Register %s", object_id);
    entity = VEDirectEntity::dynamic_build_entity_<TextSensor>(manager, name, object_id);
  }
  entity->register_id_ = register_id;
  entity->init_reg_def_(reg_def);
  manager->hex_registers_.emplace(register_id, entity);
  entity->dynamic_register();
  return entity;
}

void VEDirectEntity::set_text_label(const char *label) {
  auto text_def_it = TEXT_DEFS.find(label);
  if (text_def_it != TEXT_DEFS.end())
    this->init_text_def_(&text_def_it->second);
  this->manager->text_entities_.emplace(label, this);
}

void VEDirectEntity::set_register_id(register_id_t register_id) {
  this->register_id_ = register_id;
  this->init_reg_def_(REG_DEF::find(register_id));
  this->manager->hex_registers_.emplace(register_id, this);
}

template<typename TEntity>
TEntity *VEDirectEntity::dynamic_build_entity_(Manager *manager, const char *name, const char *object_id) {
  auto entity = new TEntity(manager);
  manager->setup_entity_name_id(entity, name, object_id);
  return entity;
}

}  // namespace m3_vedirect
}  // namespace esphome
