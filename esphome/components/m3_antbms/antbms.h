#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome {
namespace m3_antbms {

#define FRAME_POLL_READ_TIMEOUT_MS 200
#define FRAME_POLL_RESPONSE_SIZE 140

// structs for managing the bigendianness of the poll frame
typedef unsigned char byte;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short i16;
typedef unsigned int u32;
typedef int i32;

#pragma pack(push, 1)
union be_uint16 {
  typedef u16 type;
  type word;
  byte bytes[2];
  type to_le() {
    // to little endian
    return (type) ((((u16) bytes[0]) << 8) + ((u16) bytes[1]));
  }
};
union be_int16 {
  typedef i16 type;
  type word;
  byte bytes[2];
  type to_le() {
    // to little endian
    return (type) ((((u16) bytes[0]) << 8) + ((u16) bytes[1]));
  }
};
union be_uint32 {
  typedef u32 type;
  type dword;
  byte bytes[4];
  type to_le() {
    return (type) ((((u32) bytes[0]) << 24) + (((u32) bytes[1]) << 16) + (((u32) bytes[2]) << 8) + ((u32) bytes[3]));
  }
};
union be_int32 {
  typedef i32 type;
  type dword;
  byte bytes[4];
  type to_le() {
    return (type) ((((u32) bytes[0]) << 24) + (((u32) bytes[1]) << 16) + (((u32) bytes[2]) << 8) + ((u32) bytes[3]));
  }
};

union FramePoll {
  byte bytes[FRAME_POLL_RESPONSE_SIZE];
  struct {
    /* 000 */ be_uint32 Header;
    /* 004 */ be_uint16 BatteryVoltage;
    /* 006 */ be_uint16 CellVoltage[32];
    /* 070 */ be_int32 BatteryCurrent;
    /* 074 */ u8 Soc;
    /* 075 */ be_uint32 Capacity;
    /* 079 */ be_uint32 RemainingCapacity;
    /* 083 */ be_uint32 TotalCycleCapacity;
    /* 087 */ be_uint32 TimeFromBoot;  // seconds
    /* 091 */ be_int16 Temperature[6];
    /* 103 */ byte ChargeMosStatus;
    /* 104 */ byte DishargeMosStatus;
    /* 105 */ byte BalanceStatus;
    /* 106 */ be_uint16 TireLength;
    /* 108 */ be_uint16 PulsesPerWeek;
    /* 110 */ byte Relay;
    /* 111 */ be_int32 Power;
    /* 115 */ byte HighestCell;
    /* 116 */ be_uint16 HighestCellVoltage;
    /* 118 */ byte LowestCell;
    /* 119 */ be_uint16 LowestCellVoltage;
    /* 121 */ be_uint16 AverageCellVoltage;
    /* 123 */ byte CellCount;
    /* 124 */ byte Unknown[18];
    /* 138 */ be_uint16 Checksum;
  };

  bool checksum_ok() {
    u16 checksum = 0;
    for (byte *b = bytes + 4; b < Checksum.bytes; ++b)
      checksum += *b;
    return checksum == Checksum.to_le();
  }
};
#pragma pack(pop)

#define ENTITY_(type, name) \
 protected: \
  type *name##_{}; /* NOLINT */ \
\
 public: \
  void set_##name(type *name) { /* NOLINT */ \
    this->name##_ = name; \
  }

#define SENSOR(name) ENTITY_(sensor::Sensor, name)

class AntBms : public uart::UARTDevice, public PollingComponent {
 public:
  SENSOR(battery_voltage)
  SENSOR(battery_current)
  SENSOR(soc)

  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;

 protected:
  void empty_uart_buffer_();
  void read_poll_frame_();
};

}  // namespace m3_antbms
}  // namespace esphome