
#include "scs_cover.h"
#include "scs_bridge.h"
#include "esphome/core/log.h"
#include <string>

namespace esphome {
namespace scs_bridge {

#define SCH_REFRESH_INTERVAL 1000
const std::string SCH_REFRESH_NAME("refresh");
const std::string SCH_TIMEOUT_NAME("timeout");


SCSCover::SCSCover(uint8_t address, std::string name)
    : SCSDevice(address), Cover(name) {
  ESP_LOGCONFIG(SCSBridge::TAG, "SCSCover::SCSCover address=%02X name=%s", (uint32_t)address, name.c_str());
  SCSBridge::register_cover(this);
}

void SCSCover::set_max_duration(uint32_t seconds) { this->fullrun_millis_ = seconds * 1000; }

void SCSCover::command_up(uint32_t millis) {
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_CLOSING:
      this->position_millis_ -= (int32_t)(millis - this->command_millis_);
      if (this->position_millis_ < 0)
        this->position_millis_ = 0;
      this->position = float(this->position_millis_) / float(this->fullrun_millis_);
    case cover::CoverOperation::COVER_OPERATION_IDLE:
      this->command_millis_ = millis;
      this->current_operation = cover::CoverOperation::COVER_OPERATION_OPENING;
      this->publish_state(false);
      this->set_interval(SCH_REFRESH_NAME, SCH_REFRESH_INTERVAL, [this]() { this->sch_refresh_(); });
  }
}

void SCSCover::command_down(uint32_t millis) {
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_OPENING:
      this->position_millis_ += (int32_t)(millis - this->command_millis_);
      if (this->position_millis_ > this->fullrun_millis_)
        this->position_millis_ = this->fullrun_millis_;
      this->position = float(this->position_millis_) / float(this->fullrun_millis_);
    case cover::CoverOperation::COVER_OPERATION_IDLE:
      this->command_millis_ = millis;
      this->current_operation = cover::CoverOperation::COVER_OPERATION_CLOSING;
      this->publish_state(false);
      this->set_interval(SCH_REFRESH_NAME, SCH_REFRESH_INTERVAL, [this]() { this->sch_refresh_(); });
  }
}

void SCSCover::command_stop(uint32_t millis) {
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_OPENING:
      this->position_millis_ += (int32_t)(millis - this->command_millis_);
      if (this->position_millis_ >= this->fullrun_millis_)
        this->position_millis_ = this->fullrun_millis_;
      break;
    case cover::CoverOperation::COVER_OPERATION_CLOSING:
      this->position_millis_ -= (int32_t)(millis - this->command_millis_);
      if (this->position_millis_ < 0)
        this->position_millis_ = 0;
      break;
    default:
      return;
  }
  this->position = float(this->position_millis_) / float(this->fullrun_millis_);
  this->current_operation = cover::CoverOperation::COVER_OPERATION_IDLE;
  this->publish_state(false);
  this->cancel_interval(SCH_REFRESH_NAME);
  this->cancel_timeout(SCH_TIMEOUT_NAME);
  ESP_LOGD(SCSBridge::TAG, "SCSCover::command_stop : pos=%u run=%u", this->position_millis_, this->fullrun_millis_);
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
    SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_STOP, true);
  } else if (call.get_position().has_value()) {
    float pos = *call.get_position();
    if (pos == cover::COVER_OPEN) {
      SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_UP, true);
    } else if (pos == cover::COVER_CLOSED) {
      SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_DOWN, true);
    } else if (pos > this->position) {
      SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_UP, true);
      int32_t deltapos_millis = (pos - this->position) * float(this->fullrun_millis_);
      this->set_timeout(SCH_TIMEOUT_NAME, deltapos_millis, [this]() { this->sch_timeout_(); });
    } else if (pos < this->position) {
      SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_DOWN, true);
      int32_t deltapos_millis = (this->position - pos) * float(this->fullrun_millis_);
      this->set_timeout(SCH_TIMEOUT_NAME, deltapos_millis, [this]() { this->sch_timeout_(); });
    }
  }
}

void SCSCover::sch_refresh_() {
  uint32_t now = millis();
  switch (this->current_operation) {
    case cover::CoverOperation::COVER_OPERATION_OPENING:
      this->position_millis_ += (int32_t)(now - this->command_millis_);
      if (this->position_millis_ >= this->fullrun_millis_)
        this->position_millis_ = this->fullrun_millis_;
      break;
    case cover::CoverOperation::COVER_OPERATION_CLOSING:
      this->position_millis_ -= (int32_t)(now - this->command_millis_);
      if (this->position_millis_ < 0)
        this->position_millis_ = 0;
      break;
    default:
      return;
  }
  this->command_millis_ = now;
  this->position = float(this->position_millis_) / float(this->fullrun_millis_);
  this->publish_state(false);
}

void SCSCover::sch_timeout_() {
  if (this->current_operation != cover::CoverOperation::COVER_OPERATION_IDLE)
    SCSBridge::send(this->address, SCS_ADR_SCSBRIDGE, SCS_CMD_SET, SCS_VAL_COVER_STOP, true);
}

}//namespace scs_bridge
}//namespace esphome
