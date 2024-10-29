#pragma once
#include <stdint.h>
namespace m3_ve_reg {

/*typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;*/

typedef unsigned short register_id_t;
typedef unsigned char group_id_t;

// Helper to get the number of elements in static arrays
#define ARRAY_COUNT(_array) (sizeof(_array) / sizeof(_array[0]))

#pragma pack(push, 1)
/// @brief basic HEXFRAME prototype declaration
struct HEXFRAME {
  enum COMMAND : uint8_t {
    Ping = 0x1,
    Done = 0x1,  // response
    AppVersion = 0x3,
    Unknown = 0x3,  // response
    ProductId = 0x4,
    Error = 0x4,     // response
    PingResp = 0x5,  // response
    Restart = 0x6,
    Get = 0x7,
    Set = 0x8,
    Async = 0xA,
  };

  enum DATA_TYPE : uint8_t {
    STRING = 0,  // or unknown
    U8 = 1,
    U16 = 2,
    U32 = 3,
    I8 = 4,
    I16 = 5,
    I32 = 6,
    _COUNT = 7,
  };
  static const uint8_t DATA_TYPE_TO_SIZE[];
  template<typename T> static constexpr DATA_TYPE DATA_TYPE_OF();

  COMMAND command;
  union {
    uint8_t rawdata[0];
    struct {
      register_id_t register_id;
      uint8_t flags;
      union {
        uint8_t data_u8;
        int16_t data_i16;
        uint16_t data_u16;
        uint32_t data_u32;
        uint8_t data[0];
      };
    };
  };

  // member helper for generalized casting of payload data
  template<typename Tcast, typename Traw> Tcast get_data_t() { return static_cast<Tcast>(*(Traw *) this->data); }
  // static helper for generalized casting of payload data
  template<typename Tcast, typename Traw> static Tcast get_data_t(const HEXFRAME *hexframe) {
    return static_cast<Tcast>(*(Traw *) hexframe->data);
  }

  typedef int (*get_data_int_func_t)(const HEXFRAME *);
  static const get_data_int_func_t GET_DATA_AS_INT[DATA_TYPE::_COUNT];
};
#pragma pack(pop)

template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<uint8_t>() { return DATA_TYPE::U8; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<uint16_t>() { return DATA_TYPE::U16; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<uint32_t>() { return DATA_TYPE::U32; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<int8_t>() { return DATA_TYPE::I8; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<int16_t>() { return DATA_TYPE::I16; }
template<> constexpr HEXFRAME::DATA_TYPE HEXFRAME::DATA_TYPE_OF<int32_t>() { return DATA_TYPE::I32; }

}  // namespace m3_ve_reg