#pragma once

#include <string>

namespace esphome {
namespace m3_vedirect {

#define VEDIRECT_NAME_LEN 9
#define VEDIRECT_VALUE_LEN 33
#define VEDIRECT_RECORDS_COUNT 22
// Fix a (reasonable) limit to the maximum size (in raw frame bytes) of an incoming
// HEX frame so that we abort pumping data into memory when something is likely wrong
#define VEDIRECT_HEXFRAME_SIZE 64

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef unsigned short register_id_t;
typedef unsigned char group_id_t;

/// @brief  Helper class to manage HEX frames. It allows building an internal
/// binary representation and encoding/decoding
/// to the HEX format suitable for serial communication.
/// HexFrameBase works as a base implementation and needs to be initialized with
/// the correct (pre-reserved) storage to avoid usual container dynamic reallocations
struct HexFrame {
 public:
  enum Command : uint8_t {
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

#pragma pack(push, 1)
  struct Record {
    Command command;
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
  };
#pragma pack(pop)

  /// @brief Represents the format (numeric) of the data payload
  enum DataType : int8_t {
    unknown = 0,
    u8 = 1,
    u16 = 2,
    i16 = -2,
    u32 = 4,
  };

  enum DecodeResult : int8_t {
    /// @brief Decoder status ok, need more data to complete the frame
    Continue = 0,
    /// @brief Hex frame decoding completed: checksum ok
    Valid = 1,
    /// @brief Need to call 'init' before decoding
    InitError = -1,
    /// @brief Buffer too long to be stored in actual HexFrame storage
    Overflow = -2,
    /// @brief Hex frame decoding completed: checksum not ok
    ChecksumError = -3,
    /// @brief Unexpected encoding in the buffer, decoding aborted
    CodingError = -4,
    /// @brief Special case for CodingError when the input stream is 'null' terminated
    Terminated = -5,
  };

  // Generic (raw) data accessors
  inline Record *record() { return (Record *) this->rawframe_begin_; }
  inline const uint8_t *begin() const { return this->rawframe_begin_; }
  inline const uint8_t *end() const { return this->rawframe_end_; }
  inline int capacity() const { return end_of_storage() - this->rawframe_begin_; }
  inline int size() const { return this->rawframe_end_ - this->rawframe_begin_; }

  inline const char *encoded() {
    if (this->needs_encoding_())
      this->encode_();
    return encoded_begin_;
  }
  inline const char *encoded() const { return encoded_begin_; }
  inline const char *encoded_end() {
    if (this->needs_encoding_())
      this->encode_();
    return encoded_end_;
  }
  inline const char *encoded_end() const { return encoded_end_; }
  inline int encoded_size() {
    if (this->needs_encoding_())
      this->encode_();
    return encoded_end_ - encoded_begin_;
  }
  inline int encoded_size() const { return encoded_end_ - encoded_begin_; }

  // Shortcut accessors for general HEX frames data.
  // Beware these are generally not 'safe' and heavily depends
  // on frame type
  uint8_t command() const { return *this->begin(); }
  group_id_t group_id() const { return *(uint8_t *) (this->begin() + 1); }
  register_id_t register_id() const { return *(uint16_t *) (this->begin() + 1); }
  uint8_t flags() const { return *(this->begin() + 3); }
  const uint8_t *data_begin() const { return this->begin() + 4; }
  const uint8_t *data_end() const { return this->end() - 1; }
  int data_size() const { return this->size() - 5; }
  uint8_t data_u8() const { return *this->data_begin(); }
  uint16_t data_u16() const { return *(uint16_t *) this->data_begin(); }
  int16_t data_i16() const { return *(int16_t *) this->data_begin(); }
  uint32_t data_u32() const { return *(uint32_t *) this->data_begin(); }
  /// @brief Safely extracts the 'raw' payload (i.e. the data past the register id and flags)
  bool data_to_hex(std::string &hexdata) const;

  // Frame 'builders' methods
  DecodeResult decode(const char *hexdigits, bool addchecksum);

  /// @brief Builds a plain command frame payload
  /// @param command
  inline void command(Command command) {
    this->rawframe_begin_[0] = command;
    this->rawframe_end_ = this->rawframe_begin_ + 1;
    this->encode_();
  }
  /// @brief Builds a command GET register frame
  /// @param register_id
  inline void command_get(register_id_t register_id) {
    auto record = this->record();
    record->command = Command::Get;
    record->register_id = register_id;
    record->flags = 0;
    this->rawframe_end_ = this->rawframe_begin_ + 4;
    this->encode_();
  }
  template<typename DataType> void command_set(register_id_t register_id, DataType data) {
    auto record = this->record();
    record->command = Command::Set;
    record->register_id = register_id;
    record->flags = 0;
    *(DataType *) record->data = data;
    this->rawframe_end_ = this->rawframe_begin_ + 4 + sizeof(DataType);
    this->encode_();
  }

  inline void push_back(uint8_t data) {
    // warning: unchecked boundary
    *this->rawframe_end_++ = data;
  }

  // Interface(s) for custom storage implementation
  virtual const uint8_t *end_of_storage() const = 0;
  virtual const char *encoded_end_of_storage() const = 0;

 protected:
  friend class HexFrameDecoder;

  HexFrame(uint8_t *rawframe_ptr, char *encoded_ptr)
      : rawframe_begin_(rawframe_ptr),
        rawframe_end_(rawframe_ptr),
        encoded_begin_(encoded_ptr),
        encoded_end_(encoded_ptr) {}
  uint8_t *const rawframe_begin_;
  uint8_t *rawframe_end_;

  char *const encoded_begin_;
  char *encoded_end_;

  inline void invalidate_encoding_() { this->encoded_end_ = this->encoded_begin_; }
  inline bool needs_encoding_() const { return this->encoded_end_ == this->encoded_begin_; }
  void encode_();
};

/// @brief Provides a static storage implementation for HexFrame
template<std::size_t HF_DATA_SIZE> struct HexFrameT : public HexFrame {
  HexFrameT() : HexFrame(this->rawframe_, this->encoded_) {}

  const uint8_t *end_of_storage() const override { return this->rawframe_ + sizeof(this->rawframe_); }
  const char *encoded_end_of_storage() const override { return this->encoded_ + sizeof(this->encoded_); }

 protected:
  // raw buffer: contains COMMAND + DATA + CHECKSUM
  uint8_t rawframe_[1 + HF_DATA_SIZE + 1];

  // buffer for decoded/encoded hex digits:
  // :[COMMAND][DATAHIGH][DATALOW][CHECKSUMHIGH][CHECKSUMLOW]\n\0
  char encoded_[1 + 1 + HF_DATA_SIZE * 2 + 2 + 1 + 1]{":"};
};

/// @brief Helper constructor for plain 'command' frames (no payload)
struct HexFrame_Command : public HexFrameT<0> {
 public:
  HexFrame_Command(Command command) { this->command(command); }
};

/// @brief Helper constructor for plain 'command' frames (no payload)
struct HexFrame_Get : public HexFrameT<3> {
 public:
  HexFrame_Get(register_id_t register_id) { this->command_get(register_id); }
};

/// @brief Helper constructor for plain 'command' frames (no payload)
/*template <typename DataType> struct HexFrame_Set : public HexFrameT<3 + sizeof(DataType)> {
 public:
  HexFrame_Set(register_id_t register_id, DataType data) { this->command_set(register_id, data); }
};*/
struct HexFrame_Set : public HexFrameT<7> {
 public:
  template<typename DataType> HexFrame_Set(register_id_t register_id, DataType data) {
    this->command_set(register_id, data);
  }
};

class HexFrameDecoder {
 public:
  typedef HexFrame::DecodeResult Result;

  void init(HexFrame *hexframe) {
    this->checksum_ = 0x55;
    this->hinibble_ = false;
    this->hexframe_ = hexframe;
    this->rawframe_end_of_storage_ = hexframe->end_of_storage();
    hexframe->rawframe_end_ = hexframe->rawframe_begin_;
    *hexframe->rawframe_end_ = 0;
    hexframe->encoded_end_ = hexframe->encoded_begin_ + 1;
    *hexframe->encoded_end_ = 0;
  }

  /// @brief Parse an incoming byte stream of HEX digits and builds
  /// the hex frame attached to this parser through 'init'.
  /// Once any of the termination states is reached (error or frame end)
  /// the decoder needs to be reinitialized through 'init'.
  /// Termination is directly detected in the input stream by parsing
  /// an '\n' (newline) or '\0' (string termination). If the stream
  /// is not properly terminated an overflow will be detected once the
  /// input stream fills the hexframe buffers.
  /// @param hexdigit
  /// @return the parsing status 'Result' after each iteration
  Result decode(char hexdigit) {
    Result result;
    auto hexframe = this->hexframe_;
    if (!hexframe)
      return Result::InitError;
    if (hexframe->rawframe_end_ >= this->rawframe_end_of_storage_) {
      result = Result::Overflow;
      goto decode_exit;
    }
    if ((hexdigit >= '0') && (hexdigit <= '9')) {
      *hexframe->encoded_end_++ = hexdigit;
      hexdigit -= '0';
    } else if ((hexdigit >= 'A') && (hexdigit <= 'F')) {
      *hexframe->encoded_end_++ = hexdigit;
      hexdigit -= 55;
    } else if (hexdigit == '\n') {
      *hexframe->encoded_end_++ = '\n';
      result = this->checksum_ ? Result::ChecksumError : Result::Valid;
      goto decode_exit;
    } else if (hexdigit == 0) {
      // special care since we consider a 'strong' coding error when termination
      // occurs at nibble boundary
      result = this->hinibble_ ? Result::Terminated : Result::CodingError;
      goto decode_exit;
    } else {
      result = Result::CodingError;
      goto decode_exit;
    }

    if (this->hinibble_) {
      this->hinibble_ = false;
      *hexframe->rawframe_end_ = hexdigit << 4;
    } else {
      this->hinibble_ = true;
      *hexframe->rawframe_end_ |= hexdigit;
      this->checksum_ -= *hexframe->rawframe_end_++;
    }
    return Result::Continue;

  decode_exit:
    *hexframe->encoded_end_ = 0;
    this->hexframe_ = nullptr;
    return result;
  }

  inline uint8_t get_checksum() { return this->checksum_; }

 protected:
  HexFrame *hexframe_{};
  bool hinibble_;
  uint8_t checksum_;
  const uint8_t *rawframe_end_of_storage_;
};

/** VEDirect frame handler:
 * This class needs to be overriden to get notification of relevant parsing events.
 * Feed in the raw data (from serial) to 'decode' and get events for
 * hex frame -> 'on_frame_hex_'
 * text frame -> 'on_frame_text_'
 * parsing errors (overflows, checksum, etc) -> on_frame_error_
 */
class FrameHandler {
 public:
  static const char *ERR_HEXFRAME_CHECKSUM;
  static const char *ERR_HEXFRAME_CODING;
  static const char *ERR_HEXFRAME_OVERFLOW;
  static const char *ERR_TEXTFRAME_CHECKSUM;
  static const char *ERR_TEXTFRAME_NAME_OVERFLOW;
  static const char *ERR_TEXTFRAME_VALUE_OVERFLOW;
  static const char *ERR_TEXTFRAME_RECORD_OVERFLOW;

  enum FrameState {
    Idle,
    Name,
    Value,
    Checksum,
    Hex,
  };

  struct TextRecord {
    char name[VEDIRECT_NAME_LEN];
    char value[VEDIRECT_VALUE_LEN];
  };

  void reset() { this->frame_state_ = FrameState::Idle; }
  void decode(uint8_t *data_begin, uint8_t *data_end);

 private:
  //
  virtual void on_frame_hex_(const HexFrame &hexframe) {}
  virtual void on_frame_text_(TextRecord **text_records, uint8_t text_records_count) {}
  virtual void on_frame_error_(const char *message) {}

  FrameState frame_state_{FrameState::Idle};
  FrameState frame_state_backup_;

  uint8_t text_checksum_;
  // This is a statically preallocated storage for incoming records
  // TextRecord(s) will be added 'on-demand' (and reused on every new frame parsing)
  // Having 'nullptr' means the record has not been alloocated yet..simple as that.
  TextRecord *text_records_[VEDIRECT_RECORDS_COUNT]{};
  uint8_t text_records_count_;

  // buffered pointers to current text record parsing
  TextRecord *text_record_;
  char *text_record_write_;
  char *text_record_write_end_;
  inline void frame_text_name_start_() {
    TextRecord *text_record = this->text_records_[this->text_records_count_];
    if (!text_record)
      this->text_records_[this->text_records_count_] = text_record = new TextRecord;
    this->text_record_ = text_record;
    this->text_record_write_ = text_record->name;
    this->text_record_write_end_ = this->text_record_write_ + sizeof(this->text_record_->name);
  }
  inline void frame_text_value_start_() {
    // current text_record_ already in place since we were parsing the name
    this->text_record_write_ = this->text_record_->value;
    this->text_record_write_end_ = this->text_record_write_ + sizeof(this->text_record_->value);
  }

  HexFrameT<VEDIRECT_HEXFRAME_SIZE> hexframe_;
  HexFrameDecoder hexframe_decoder_;

  inline void frame_hex_start_() {
    this->hexframe_decoder_.init(&hexframe_);
    this->frame_state_backup_ = this->frame_state_;
    this->frame_state_ = FrameState::Hex;
  }
};

}  // namespace m3_vedirect
}  // namespace esphome
