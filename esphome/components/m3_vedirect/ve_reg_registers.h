#pragma once

// clang-format off

#define REGISTERS_COMMON(MACRO) \
  MACRO##_ENUM(0x200, DEVICE_MODE, READ_WRITE) \
  MACRO##_ENUM(0x201, DEVICE_STATE, READ_ONLY) \
  MACRO##_NUMERIC(0x2201, AC_OUT_CURRENT, READ_ONLY, int16_t, D_1, A)

// clang-format on