#pragma once

namespace esphome {
namespace m3_victron_ble_ir {

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef signed short int16_t;
typedef unsigned int u_int32_t;
typedef signed int int32_t;

// Max documented message size is 16 byte. Maximum length of a record is 20 bytes = 4 byte header
// (VICTRON_BLE_RECORD_BASE minus VICTRON_BLE_MANUFACTURER_DATA) + 16 byte payload
#define VICTRON_ENCRYPTED_DATA_MAX_SIZE 16

// Helper to get the number of elements in static arrays
#define ARRAY_COUNT(_array) (sizeof(_array) / sizeof(_array[0]))

}  // namespace m3_victron_ble_ir
}  // namespace esphome