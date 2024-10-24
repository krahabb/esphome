#pragma once
#include <string>
#include <vector>
#include "ve_reg_enums.h"
#include "ve_reg_registers.h"

namespace ve_reg {

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

// clang-format off

// clang-format on

#pragma pack(push, 1)

/// @brief Base class acting as 'enum helper' for registers containing enumerated values
struct ENUM_DEF {
 public:
  // allow an easy parameterization of the underlying enum representation
  // we'll start with a believe of always being uint8 but we might need to templatize this struct
  // should some enum registers hold bigger data representations
  typedef uint8_t data_type;

  struct LOOKUP_DEF {
    data_type value;
    const char *label;
    bool operator<(const data_type &value) const { return this->value < value; }
  };

  typedef const char *(*lookup_func_t)(data_type value);

 protected:
  /// @brief Lookup the raw enum 'value' and returns the string description
  /// @brief according to the enum label definition.
  static const char *lookup(data_type value, const LOOKUP_DEF *lookup, const LOOKUP_DEF *lookup_end);
};

// declare the enum helpers structs for ENUM registers
#define _ENUMS_ITEM(enum, value) enum = value
#define DECLARE_ENUMS_NUMERIC(...)
#define DECLARE_ENUMS_ENUM(register_id, label, access) \
  struct VE_REG_##label##_ENUM : public ENUM_DEF { \
   public: \
    enum : data_type { ENUM_##label(_ENUMS_ITEM) }; \
    static const LOOKUP_DEF LOOKUP[]; \
    static const LOOKUP_DEF *const LOOKUP_END; \
    static const char *lookup(data_type value) { return ENUM_DEF::lookup(value, LOOKUP, LOOKUP_END); } \
  };

REGISTERS_COMMON(DECLARE_ENUMS)
#undef DECLARE_ENUMS_ENUM
#undef DECLARE_ENUMS_NUMERIC
#undef _ENUMS_ITEM

struct REG_DEF {
#define _DECLARE_REG_LABEL_ENUM(register_id, label, ...) label,
#define _DECLARE_REG_LABEL_NUMERIC(register_id, label, ...) label,
  enum LABEL : uint16_t { REGISTERS_COMMON(_DECLARE_REG_LABEL) };
#undef _DECLARE_REG_LABEL_ENUM
#undef _DECLARE_REG_LABEL_NUMERIC

  /// @brief Together with SUBCLASS defines the data semantics of this entity
  enum CLASS : u_int8_t {
    BOOLEAN,
    BITMASK,  // represents a set of bit flags
    ENUM,     // enumeration data
    NUMERIC,  // numeric data (either signed or unsigned)
  };

  enum DATA_TYPE : uint8_t {
    STRING = 0,
    U8 = 1,
    U16 = 2,
    U32 = 4,
    I8 = 5,
    I16 = 6,
    I32 = 8,
  };
  template<typename T> static constexpr DATA_TYPE DATA_TYPE_OF();

  enum ACCESS : u_int8_t {
    READ_ONLY = 0,
    READ_WRITE = 1,
  };

  // configuration symbols for numeric sensors
  enum UNIT : u_int8_t {
    NONE,
    A,
    V,
    VA,
    W,
    Ah,
    kWh,
    SOC_PERCENTAGE,
    minute,
    CELSIUS,
  };
  static const char *UNITS[];

  enum DIGITS : u_int8_t {
    D_0 = 0,
    D_1 = 1,
    D_2 = 2,
    D_3 = 3,
  };
  static const float DIGITS_TO_SCALE[4];
  typedef float (*numeric_to_float_func_t)(const uint8_t *rawdata);
  template<typename T, DIGITS digits> inline static float numeric_to_float_t(const uint8_t *rawdata) {
    return *(T *) (rawdata) *DIGITS_TO_SCALE[digits];
  };

  const register_id_t register_id;
  const char *label;
  const CLASS cls : 3;
  const ACCESS access : 1;
  const DATA_TYPE data_type : 4;

  union {
    const ENUM_DEF::lookup_func_t enum_lookup;
    struct {
      const numeric_to_float_func_t numeric_to_float;
      DIGITS digits : 2;
      UNIT unit : 4;
    };
  };

  static const REG_DEF DEFS[];
  bool operator<(const register_id_t register_id) const { return this->register_id < register_id; }
  static const REG_DEF *find(register_id_t register_id);

  /// @brief Constructor for NUMERIC registers definitions
  REG_DEF(register_id_t register_id, const char *label, ACCESS access, DATA_TYPE data_type, DIGITS digits, UNIT unit,
          numeric_to_float_func_t numeric_to_float)
      : register_id(register_id),
        label(label),
        cls(CLASS::NUMERIC),
        access(access),
        data_type(data_type),
        numeric_to_float(numeric_to_float),
        digits(digits),
        unit(unit) {}

  /// @brief Constructor for ENUM registers definitions
  REG_DEF(register_id_t register_id, const char *label, ACCESS access, ENUM_DEF::lookup_func_t enum_lookup)
      : register_id(register_id),
        label(label),
        cls(CLASS::ENUM),
        access(access),
        data_type(DATA_TYPE::U8),
        enum_lookup(enum_lookup) {}

 protected:
};

template<> constexpr REG_DEF::DATA_TYPE REG_DEF::DATA_TYPE_OF<uint8_t>() { return DATA_TYPE::U8; }
template<> constexpr REG_DEF::DATA_TYPE REG_DEF::DATA_TYPE_OF<uint16_t>() { return DATA_TYPE::U16; }
template<> constexpr REG_DEF::DATA_TYPE REG_DEF::DATA_TYPE_OF<uint32_t>() { return DATA_TYPE::U32; }
template<> constexpr REG_DEF::DATA_TYPE REG_DEF::DATA_TYPE_OF<int8_t>() { return DATA_TYPE::I8; }
template<> constexpr REG_DEF::DATA_TYPE REG_DEF::DATA_TYPE_OF<int16_t>() { return DATA_TYPE::I16; }
template<> constexpr REG_DEF::DATA_TYPE REG_DEF::DATA_TYPE_OF<int32_t>() { return DATA_TYPE::I32; }

#pragma pack(pop)

}  // namespace ve_reg