#pragma once
#include <esphome/core/datatypes.h>
#include <vector>

namespace esphome {
namespace m3_vedirect {

typedef uint16_t register_id_t;
typedef uint8_t group_id_t;

/// @brief Helper class to manage HEX frames. It allows building an internal
/// @brief binary representation (based off vector<uint8_t>) and encoding/decoding
/// @brief to the HEX format suitable for serial communication.
/// @brief In actual implementation, the final checksum might or might not be
/// @brief included in the array.
class HexFrame : protected std::vector<uint8_t> {
 public:
  /// @brief Represents the format (numeric) of the data payload
  enum Command : int8_t {
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

  /// @brief Represents the format (numeric) of the data payload
  enum DataType : int8_t {
    unknown = 0,
    u8 = 1,
    u16 = 2,
    i16 = -2,
    u32 = 4,
  };

  bool valid() const { return this->valid_; }

  // Shortcut accessors for general HEX frames data.
  // Beware these are generally not 'safe' and heavily depends
  // on frame type
  uint8_t command() const { return *this->data(); }
  group_id_t group_id() const { return *(uint8_t *) (this->data() + 1); }
  register_id_t register_id() const { return *(uint16_t *) (this->data() + 1); }
  uint8_t flags() const { return *(this->data() + 3); }
  const uint8_t *data_ptr() const { return this->data() + 4; }
  size_t data_size() const { return this->size() - 5; }
  uint8_t data_u8() const { return *this->data_ptr(); }
  uint16_t data_u16() const { return *(uint16_t *) this->data_ptr(); }
  int16_t data_i16() const { return *(int16_t *) this->data_ptr(); }
  uint32_t data_u32() const { return *(uint32_t *) this->data_ptr(); }
  /// @brief Safely extracts the 'raw' payload (i.e. the data past the register id and flags)
  /// @return the hex encoded raw payload
  std::string data_hex() const;

 protected:
  /// @brief Construct from HEX char buffer. Parsing
  /// @brief will terminate as soon as a non HEX char is encountered
  /// @param buf
  HexFrame() : valid_(false) {}
  HexFrame(const char *buf);

  bool valid_;
};

class HexRxFrame : public HexFrame {
 public:
  /// @brief cached pointer to the original buffer (warning!)
  const char *const src;
  HexRxFrame(const char *buf) : HexFrame(buf), src(buf) {}

 protected:
};

class HexTxFrame : public HexFrame {
 public:
  HexTxFrame(const char *buf) : HexFrame(buf) {}
  HexTxFrame(Command command) { this->push_back(command); }
  const std::string &raw_frame() const { return this->raw_frame_; }

  const std::string &encode() const;

 protected:
  std::string raw_frame_;
};

}  // namespace m3_vedirect
}  // namespace esphome
