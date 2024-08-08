#pragma once
#include "esphome/core/entity_base.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

#include "hexframe.h"

namespace esphome {
namespace m3_vedirect {

#define NO_REGISTER_ID 0
#define NO_TEXT_NAME nullptr

class Manager;

/// @brief Provides an abstraction for an HEX register id. In case of simple registers
/// @brief it also represents the associated entities while, for registers carrying
/// @brief multiple data parameters, it takes the responsibility of splitting those data
/// @brief and forwarding to the correct entity
class HexRegister {
 public:
  Manager *const manager;

  register_id_t id;
  HexRegister(Manager *manager, register_id_t id);

  virtual void dynamic_register(){};

  virtual void parse_hex_value(const HexFrame *hexframe){};

 protected:
};

class VEDirectEntity {
 public:
  /// @brief Called when an entity is dynamically initialized by the Manager loop.
  /// @brief This will in turn call the proper register function against App/api
  virtual void dynamic_register(){};
};

class HFEntity : public HexRegister, public VEDirectEntity {
 public:
 protected:
  HFEntity(Manager *manager, register_id_t id) : HexRegister(manager, id) {}
};

struct TFEntityDef {
  /// @brief the VE.Direct record name for entities advertised over TEXT frames
  const char *const label;
  /// @brief Friendly name of the entity used to automatically configure ESPHome entity name
  const char *const description;
};

class TFEntity : public VEDirectEntity {
 public:
  const char *const label;

  /// @brief store the current text value from VE.Direct TEXT frame
  /// @param value the raw data carried by VE.Direct
  virtual void parse_text_value(const char *text_value){};

 protected:
  TFEntity(Manager *manager, const char *label);
};

}  // namespace m3_vedirect
}  // namespace esphome
