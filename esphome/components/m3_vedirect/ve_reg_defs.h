#pragma once
#include <string>
#include <vector>
#include "ve_reg.h"
#include "ve_reg_enums.h"
#include "ve_reg_registers.h"

namespace m3_ve_reg {

// clang-format off

// clang-format on

#pragma pack(push, 1)

/// @brief Base class acting as 'enum helper' for registers containing enumerated values
struct ENUM_DEF {
 public:
  // allow an easy parameterization of the underlying enum representation
  // we'll start with a believe of always being uint8 but we might need to templatize this struct
  // should some enum registers hold bigger data representations
  typedef uint8_t enum_type;
  static constexpr enum_type VALUE_UNKNOWN = 0xFF;

  struct LOOKUP_DEF {
    enum_type value;
    const char *label;
    bool operator<(const enum_type &value) const { return this->value < value; }
  };

  struct LOOKUP_RESULT {
    int index;
    LOOKUP_DEF *lookup_def;
    bool added;
  };

  typedef const char *(*lookup_func_t)(enum_type value);

  std::vector<LOOKUP_DEF> LOOKUPS;
  ENUM_DEF(std::initializer_list<LOOKUP_DEF> initializer_list) : LOOKUPS(initializer_list) {}

  /// @brief Lookups the label associated with value in current definitions
  /// @param value
  /// @return nullptr if no label definition for value
  const char *lookup_label(enum_type value);
  /// @brief Lookups a matching label in current definitions
  /// @param label
  /// @return nullptr if no lookup definition
  const LOOKUP_DEF *lookup_value(const char *label);

  /// @brief Lookups (eventually adding) the label associated with value in current definitions
  /// @param value
  /// @return the whole lookup definition with additional context in LOOKUP_RESULT
  LOOKUP_RESULT get_lookup(enum_type value);
};

/// @brief Helper for registers carrying BITMASK class data. This is implemented mainly as an enumeration
/// but will add helpers for treating the enums so defined as bitmask values
struct BITMASK_DEF : public ENUM_DEF {
 public:
  typedef uint32_t bitmask_t;
  static constexpr bitmask_t VALUE_UNKNOWN = 0xFFFFFFFF;
  BITMASK_DEF(std::initializer_list<LOOKUP_DEF> initializer_list) : ENUM_DEF(initializer_list) {}
};

// declare the enum helpers structs for ENUM registers
#define _ENUMS_ITEM(enum, value) enum = value
#define DECLARE_ENUMS_BITMASK(register_id, label, ...) \
  struct VE_REG_##label##_BITMASK : public BITMASK_DEF { \
   public: \
    enum : enum_type { BITMASK_##label(_ENUMS_ITEM) }; \
  }; \
  extern BITMASK_DEF VE_REG_##label##_BITMASK_DEF;
#define DECLARE_ENUMS_ENUM(register_id, label, ...) \
  struct VE_REG_##label##_ENUM : public ENUM_DEF { \
   public: \
    enum : enum_type { ENUM_##label(_ENUMS_ITEM) }; \
  }; \
  extern ENUM_DEF VE_REG_##label##_ENUM_DEF;
#define DECLARE_ENUMS_NUMERIC(...)
REGISTERS_COMMON(DECLARE_ENUMS)
#undef DECLARE_ENUMS_BITMASK
#undef DECLARE_ENUMS_ENUM
#undef DECLARE_ENUMS_NUMERIC
#undef _ENUMS_ITEM

struct REG_DEF {
#define _DECLARE_REG_LABEL_BITMASK(register_id, label, ...) label,
#define _DECLARE_REG_LABEL_ENUM(register_id, label, ...) label,
#define _DECLARE_REG_LABEL_NUMERIC(register_id, label, ...) label,
  enum LABEL : uint16_t { REGISTERS_COMMON(_DECLARE_REG_LABEL) };
#undef _DECLARE_REG_LABEL_BITMASK
#undef _DECLARE_REG_LABEL_ENUM
#undef _DECLARE_REG_LABEL_NUMERIC

  /// @brief Together with SUBCLASS defines the data semantics of this entity
  enum CLASS : u_int8_t {
    UNKNOWN,
    BITMASK,  // represents a set of bit flags
    BOOLEAN,
    ENUM,     // enumeration data
    NUMERIC,  // numeric data (either signed or unsigned)
  };

  enum ACCESS : u_int8_t {
    READ_ONLY = 0,
    READ_WRITE = 1,
  };

  typedef HEXFRAME::DATA_TYPE DATA_TYPE;

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
  const char *const label;
  const CLASS cls : 3;
  const ACCESS access : 1;
  const DATA_TYPE data_type : 3;

  union {
    ENUM_DEF *const enum_def;
    struct {
      numeric_to_float_func_t const numeric_to_float;
      DIGITS const digits : 2;
      UNIT const unit : 4;
    };
  };

  static const REG_DEF DEFS[];
  bool operator<(const register_id_t register_id) const { return this->register_id < register_id; }
  static const REG_DEF *find(register_id_t register_id);

  REG_DEF(register_id_t register_id)
      : register_id(register_id),
        label(nullptr),
        cls(CLASS::UNKNOWN),
        access(ACCESS::READ_ONLY),
        data_type(DATA_TYPE::STRING) {}
  /// @brief Constructor for BITMASK registers definitions
  REG_DEF(register_id_t register_id, const char *label, ACCESS access, DATA_TYPE data_type, ENUM_DEF *enum_def)
      : register_id(register_id),
        label(label),
        cls(CLASS::BITMASK),
        access(access),
        data_type(data_type),
        enum_def(enum_def) {}
  /// @brief Constructor for ENUM registers definitions
  REG_DEF(register_id_t register_id, const char *label, ACCESS access, ENUM_DEF *enum_def)
      : register_id(register_id),
        label(label),
        cls(CLASS::ENUM),
        access(access),
        data_type(DATA_TYPE::U8),
        enum_def(enum_def) {}
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

 protected:
};

#pragma pack(pop)

}  // namespace m3_ve_reg