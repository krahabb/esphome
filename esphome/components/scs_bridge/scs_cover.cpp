
#include "scs_cover.h"
#include "scs_bridge.h"
#include "esphome/core/log.h"
#include <string>

namespace esphome {
namespace scs_bridge {


SCSCover::SCSCover(uint8_t address, std::string name)
    : SCSDevice(address), Cover(name), fullrun_micros_(30000000) {
  ESP_LOGCONFIG(SCSBridge::TAG, "SCSCover::SCSCover address=%02X name=%s", (uint32_t)address, name.c_str());
}

void SCSCover::command_up(uint32_t micros) {
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_CLOSING:
      this->position_micros_ -= micros - this->command_micros_;
      if (this->position_micros_ < 0)
        this->position_micros_ = 0;
    case cover::CoverOperation::COVER_OPERATION_IDLE:
      this->command_micros_ = micros;
      this->position = float(this->position_micros_) / float(this->fullrun_micros_);
      this->current_operation = cover::CoverOperation::COVER_OPERATION_OPENING;
      this->publish_state(false);
  }
}

void SCSCover::command_down(uint32_t micros) {
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_OPENING:
      this->position_micros_ += micros - this->command_micros_;
      if (this->position_micros_ > this->fullrun_micros_)
        this->position_micros_ = this->fullrun_micros_;
    case cover::CoverOperation::COVER_OPERATION_IDLE:
      this->command_micros_ = micros;
      this->position = float(this->position_micros_) / float(this->fullrun_micros_);
      this->current_operation = cover::CoverOperation::COVER_OPERATION_CLOSING;
      this->publish_state(false);
  }
}

void SCSCover::command_stop(uint32_t micros) {
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_OPENING:
      this->position_micros_ += micros - this->command_micros_;
      if (this->position_micros_ >= this->fullrun_micros_)
        this->position_micros_ = this->fullrun_micros_;
      break;
    case cover::CoverOperation::COVER_OPERATION_CLOSING:
      this->position_micros_ -= micros - this->command_micros_;
      if (this->position_micros_ < 0)
        this->position_micros_ = 0;
      break;
    default:
      return;
  }
  this->position = float(this->position_micros_) / float(this->fullrun_micros_);
  this->current_operation = cover::CoverOperation::COVER_OPERATION_IDLE;
  this->publish_state(false);
}

void SCSCover::loop_refresh(uint32_t micros) {
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_OPENING:
      this->position_micros_ += micros - this->command_micros_;
      if (this->position_micros_ >= this->fullrun_micros_)
        this->position_micros_ = this->fullrun_micros_;
      break;
    case cover::CoverOperation::COVER_OPERATION_CLOSING:
      this->position_micros_ -= micros - this->command_micros_;
      if (this->position_micros_ < 0)
        this->position_micros_ = 0;
      break;
    default:
      return;
  }
  this->command_micros_ = micros;
  this->position = float(this->position_micros_) / float(this->fullrun_micros_);
  this->publish_state(false);
}

cover::CoverTraits SCSCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_is_assumed_state(true);
  traits.set_supports_position(true);
  traits.set_supports_tilt(false);
  return traits;
}

void SCSCover::control(const cover::CoverCall &call) {
  // This will be called every time the user requests a state change.
  if (call.get_stop()) {
    SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_STOP);
  } else if (call.get_position().has_value()) {
    float pos = *call.get_position();
    if (pos >= this->position)
      SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_UP);
    else
      SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_DOWN);
  }
}


}//namespace scs_bridge
}//namespace esphome
