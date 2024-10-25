#pragma once
namespace m3_ve_reg {

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef unsigned short register_id_t;
typedef unsigned char group_id_t;

// Helper to get the number of elements in static arrays
#define ARRAY_COUNT(_array) (sizeof(_array) / sizeof(_array[0]))

}  // namespace m3_ve_reg