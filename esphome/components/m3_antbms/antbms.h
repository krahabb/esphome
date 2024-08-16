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
#define FRAME_POLL_CELLS_COUNT 32
#define FRAME_POLL_TEMPERATURES_COUNT 6

#define FREE_HEAP_CHECK_TIMEOUT_MS 60000

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

inline u16 be_u16(const byte *bytes) { return (u16) ((((u16) bytes[0]) << 8) + ((u16) bytes[1])); }
inline i16 be_i16(const byte *bytes) { return (i16) ((((u16) bytes[0]) << 8) + ((u16) bytes[1])); }
inline u32 be_u32(const byte *bytes) {
  return (u32) ((((u32) bytes[0]) << 24) + (((u32) bytes[1]) << 16) + (((u32) bytes[2]) << 8) + ((u32) bytes[3]));
}
inline i32 be_i32(const byte *bytes) {
  return (i32) ((((u32) bytes[0]) << 24) + (((u32) bytes[1]) << 16) + (((u32) bytes[2]) << 8) + ((u32) bytes[3]));
}

inline float be_u8_to_float(const byte *bytes) { return (float) bytes[0]; }
inline float be_u16_to_float(const byte *bytes) { return (float) be_u16(bytes); }
inline float be_i16_to_float(const byte *bytes) { return (float) be_i16(bytes); }
inline float be_u32_to_float(const byte *bytes) { return (float) be_u32(bytes); }
inline float be_i32_to_float(const byte *bytes) { return (float) be_i32(bytes); }

union FramePoll {
  byte bytes[FRAME_POLL_RESPONSE_SIZE];
  struct {
    /* 000 */ be_uint32 Header;
    /* 004 */ be_uint16 battery_voltage;
    /* 006 */ be_uint16 cell_voltage[FRAME_POLL_CELLS_COUNT];
    /* 070 */ be_int32 battery_current;
    /* 074 */ u8 soc;
    /* 075 */ be_uint32 capacity_total;
    /* 079 */ be_uint32 capacity_remaining;
    /* 083 */ be_uint32 TotalCycleCapacity;
    /* 087 */ be_uint32 TimeFromBoot;  // seconds
    /* 091 */ be_int16 temperature[FRAME_POLL_TEMPERATURES_COUNT];
    /* 103 */ byte charge_mos_status;
    /* 104 */ byte discharge_mos_status;
    /* 105 */ byte balance_status;
    /* 106 */ be_uint16 TireLength;
    /* 108 */ be_uint16 PulsesPerWeek;
    /* 110 */ byte Relay;
    /* 111 */ be_int32 battery_power;
    /* 115 */ byte HighestCell;
    /* 116 */ be_uint16 cell_high_voltage;
    /* 118 */ byte LowestCell;
    /* 119 */ be_uint16 cell_low_voltage;
    /* 121 */ be_uint16 AverageCellVoltage;
    /* 123 */ u8 cell_count;
    /* 124 */ byte Unknown[14];
    /* 138 */ be_uint16 Checksum;
  };

  bool checksum_ok() {
    u16 checksum = 0;
    byte *b_end = bytes + sizeof(bytes) - 2;
    for (byte *b = bytes + 4; b < b_end; ++b)
      checksum += *b;
    return checksum == Checksum.to_le();
  }
};
#pragma pack(pop)

#define ENTITY(type, name) \
 protected: \
  type *name##_{}; /* NOLINT */ \
\
 public: \
  void set_##name(type *_entity) { /* NOLINT */ \
    this->name##_ = _entity; \
  }

#define ENTITY_ARRAY(type, name, max_count) \
 public: \
  void set_##name(type *_entity, int count);

class AntBms;

/// @brief Root of *SensorConfig classes: these will generalize the frame parsing based off
/// the format (and scale) of the encoded data
class SensorConfig {
 public:
  EntityBase *entity;
  const int offset;

  SensorConfig(AntBms *_antbms, const char *_name, EntityBase *_entity, int _offset);

  virtual void parse(const FramePoll &frame) = 0;
};

/// @brief Config class for plain 'byte' type sensors (simplified over FloatSensorConfig)
class ByteSensorConfig : public SensorConfig {
 public:
  typedef float (*convert_func_type)(const byte *);
  sensor::Sensor *sensor;
  ByteSensorConfig(AntBms *_antbms, const char *_name, sensor::Sensor *_sensor, int _offset)
      : SensorConfig(_antbms, _name, _sensor, _offset), sensor(_sensor) {}

  void parse(const FramePoll &frame) override {
    float value = frame.bytes[this->offset];
    if (value != this->sensor->get_raw_state())
      this->sensor->publish_state(value);
  }
};
#define BYTE_SENSOR_CONFIG(name, _offset) \
  void set_##name(sensor::Sensor *_sensor) { /* NOLINT */ \
    this->sensors_.push_back(new ByteSensorConfig(this, #name, _sensor, _offset)); \
  }

class FloatSensorConfig : public SensorConfig {
 public:
  typedef float (*convert_func_type)(const byte *);
  sensor::Sensor *sensor;
  const convert_func_type convert_func;
  const float scale;
  FloatSensorConfig(AntBms *_antbms, const char *_name, sensor::Sensor *_sensor, int _offset,
                    convert_func_type _convert_func, float _scale)
      : SensorConfig(_antbms, _name, _sensor, _offset), sensor(_sensor), convert_func(_convert_func), scale(_scale) {}

  void parse(const FramePoll &frame) override {
    float value = this->convert_func(frame.bytes + this->offset) * this->scale;
    if (value != this->sensor->get_raw_state())
      this->sensor->publish_state(value);
  }
};
#define FLOAT_SENSOR_CONFIG(name, _offset, _convert_func, _scale) \
  void set_##name(sensor::Sensor *_sensor) { /* NOLINT */ \
    this->sensors_.push_back(new FloatSensorConfig(this, #name, _sensor, _offset, _convert_func, _scale)); \
  }

/// @brief Specialized parser for cell voltages in order to 'statically' cut down the least
/// significant digit (too noisy)
class CellSensorConfig : public FloatSensorConfig {
 public:
  static const char *OBJECT_ID;
  CellSensorConfig(AntBms *_antbms, sensor::Sensor *_sensor, int _offset)
      : FloatSensorConfig(_antbms, OBJECT_ID, _sensor, _offset, be_u16_to_float, 0.001f) {}

  void parse(const FramePoll &frame) override {
    float value = ((be_u16(frame.bytes + this->offset) + 5) / 10) * 0.01f;
    if (value != this->sensor->get_raw_state())
      this->sensor->publish_state(value);
  }
};
#define CELL_SENSOR_CONFIG(name, _offset) \
  void set_##name(sensor::Sensor *_sensor) { /* NOLINT */ \
    this->sensors_.push_back(new CellSensorConfig(this, _sensor, _offset)); \
  }

class TextSensorConfig : public SensorConfig {
 public:
  static const char *const CHARGE_MOS_MAP[];
  static const int CHARGE_MOS_MAP_SIZE;
  static const char *const DISCHARGE_MOS_MAP[];
  static const int DISCHARGE_MOS_MAP_SIZE;
  static const char *const BALANCE_MAP[];
  static const int BALANCE_MAP_SIZE;

  text_sensor::TextSensor *sensor;
  const char *const *map;
  const int map_size;
  TextSensorConfig(AntBms *_antbms, const char *_name, text_sensor::TextSensor *_sensor, int _offset,
                   const char *const _map[], int _map_size)
      : SensorConfig(_antbms, _name, _sensor, _offset), sensor(_sensor), map(_map), map_size(_map_size) {}

  void parse(const FramePoll &frame) override {
    byte byte_value = frame.bytes[this->offset];
    char buf[10];
    const char *p_char_value;
    if (byte_value < this->map_size) {
      p_char_value = this->map[byte_value];
    } else {
      sprintf(buf, "%i", byte_value);
      p_char_value = buf;
    }
    std::string value(p_char_value);
    if (value != this->sensor->get_raw_state())
      this->sensor->publish_state(value);
  }
};
#define TEXT_SENSOR_CONFIG(name, _offset, _map, _map_size) \
  void set_##name(text_sensor::TextSensor *_sensor) { /* NOLINT */ \
    this->sensors_.push_back(new TextSensorConfig(this, #name, _sensor, _offset, _map, _map_size)); \
  }

class AntBms : public uart::UARTDevice, public PollingComponent {
 public:
  FLOAT_SENSOR_CONFIG(battery_voltage, 4, be_u16_to_float, 0.1f)
  FLOAT_SENSOR_CONFIG(battery_current, 70, be_i32_to_float, 0.1f)
  FLOAT_SENSOR_CONFIG(battery_power, 111, be_i32_to_float, 1.f)
  BYTE_SENSOR_CONFIG(soc, 74)
  FLOAT_SENSOR_CONFIG(capacity_remaining, 79, be_u32_to_float, 0.000001f)
  ENTITY_ARRAY(sensor::Sensor, cell_voltage, FRAME_POLL_CELLS_COUNT)
  CELL_SENSOR_CONFIG(cell_high_voltage, 116)
  CELL_SENSOR_CONFIG(cell_low_voltage, 119)
  BYTE_SENSOR_CONFIG(cell_count, 123)
  ENTITY_ARRAY(sensor::Sensor, temperature, FRAME_POLL_TEMPERATURES_COUNT)
  TEXT_SENSOR_CONFIG(charge_mos_status, 103, TextSensorConfig::CHARGE_MOS_MAP, TextSensorConfig::CHARGE_MOS_MAP_SIZE)
  TEXT_SENSOR_CONFIG(discharge_mos_status, 104, TextSensorConfig::DISCHARGE_MOS_MAP,
                     TextSensorConfig::DISCHARGE_MOS_MAP_SIZE)
  TEXT_SENSOR_CONFIG(balance_status, 105, TextSensorConfig::BALANCE_MAP, TextSensorConfig::BALANCE_MAP_SIZE)

  ENTITY(sensor::Sensor, battery_energy)
  ENTITY(binary_sensor::BinarySensor, link_connected)
  ENTITY(sensor::Sensor, memory_free)

  const char *get_object_id_prefix() { return this->object_id_prefix_; }
  void set_object_id_prefix(const char *_prefix) { this->object_id_prefix_ = _prefix; }

  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;

 protected:
  const char *object_id_prefix_{};

  long battery_energy_last_time{0};
  float battery_energy_last_power{0.f};
  float battery_energy_value{0.f};

  std::vector<SensorConfig *> sensors_;
  void empty_uart_buffer_();
  void read_poll_frame_();
  void free_heap_check_();
};

}  // namespace m3_antbms
}  // namespace esphome