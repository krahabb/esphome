#pragma once

namespace esphome {
namespace m3_victron_ble_ir {

/// @brief Abstract base class for all of the entities
class VictronBleIrEntity {
 public:
  typedef void *(*parse_func)(void *record);

  void parse(void *record) { this->parse_(record); }

 protected:
  parse_func parse_;
};

}  // namespace m3_victron_ble_ir
}  // namespace esphome