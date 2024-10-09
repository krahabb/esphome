#include "hexframe.h"
#include <string.h>

namespace esphome {
namespace m3_vedirect {

const char HEX_DIGITS_MAP[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

bool HexFrame::data_to_hex(std::string &hexdata) const {
  int encoded_size = this->encoded_end_ - this->encoded_begin_;
  if (encoded_size > 11) {
    hexdata.assign(this->encoded_begin_ + 8, this->encoded_end_ - 3);
    return true;
  }
  return false;
}

HexFrame::DecodeResult HexFrame::decode(const char *hexdigits, bool addchecksum) {
  HexFrameDecoder decoder;
  decoder.init(this);

  for (;;) {
    switch (auto result = decoder.decode(*hexdigits++)) {
      case DecodeResult::Continue:
        continue;
      case DecodeResult::Terminated:
        if (addchecksum) {
          uint8_t checksum = decoder.get_checksum();
          this->push_back(checksum);
          *this->encoded_end_++ = HEX_DIGITS_MAP[checksum >> 4];
          *this->encoded_end_++ = HEX_DIGITS_MAP[checksum & 0x0F];
          *this->encoded_end_++ = '\n';
          *this->encoded_end_ = 0;
          return DecodeResult::Valid;
        } else {
          *this->encoded_end_++ = '\n';
          *this->encoded_end_ = 0;
          return DecodeResult::Valid;
        }
      default:
        return result;
    }
  }
}

void HexFrame::encode_() {
  this->encoded_end_ = this->encoded_begin_;
  int data_size = this->size();
  if (data_size > 0) {
    int encoded_size = 1 + 1 + (data_size - 1) * 2 + 2 + 2;
    if (encoded_size <= (this->encoded_end_of_storage() - this->encoded_begin_)) {
      *this->encoded_end_++ = ':';
      const uint8_t *data = this->begin();
      uint8_t checksum = 0x55 - *data;
      *this->encoded_end_++ = HEX_DIGITS_MAP[(*data) & 0x0F];
      while (++data < this->end()) {
        *this->encoded_end_++ = HEX_DIGITS_MAP[(*data) >> 4];
        *this->encoded_end_++ = HEX_DIGITS_MAP[(*data) & 0x0F];
        checksum -= *data;
      }
      if (checksum) {
        // in case the rawframe doesn't (yet) hold the checksum
        *this->encoded_end_++ = HEX_DIGITS_MAP[checksum >> 4];
        *this->encoded_end_++ = HEX_DIGITS_MAP[checksum & 0x0F];
      }
      *this->encoded_end_++ = '\n';
    }
  }
  *this->encoded_end_ = 0;
}

const char *FrameHandler::ERR_HEXFRAME_CHECKSUM = "HEX FRAME: checksum error";
const char *FrameHandler::ERR_HEXFRAME_CODING = "HEX FRAME: coding error";
const char *FrameHandler::ERR_HEXFRAME_OVERFLOW = "HEX FRAME: overflow error";
const char *FrameHandler::ERR_TEXTFRAME_CHECKSUM = "TEXT FRAME: invalid checksum";
const char *FrameHandler::ERR_TEXTFRAME_NAME_OVERFLOW = "TEXT FRAME: overflow NAME";
const char *FrameHandler::ERR_TEXTFRAME_VALUE_OVERFLOW = "TEXT FRAME: overflow VALUE";
const char *FrameHandler::ERR_TEXTFRAME_RECORD_OVERFLOW = "TEXT FRAME: overflow RECORD COUNT";

void FrameHandler::decode(uint8_t *data_begin, uint8_t *data_end) {
  uint8_t data;
handle_state:
  switch (this->frame_state_) {
    case FrameState::Name:
    handle_state_name:
      while (data_begin < data_end) {
        data = *data_begin++;
        switch (data) {
          case '\t':  // end of name
            this->text_checksum_ += '\t';
            *this->text_record_write_ = 0;
            if (strcmp(this->text_record_->name, "Checksum")) {
              this->frame_text_value_start_();
              this->frame_state_ = FrameState::Value;
              goto handle_state_value;
            } else {  // the Checksum record indicates a EOF
              this->frame_state_ = FrameState::Checksum;
              goto handle_state_checksum;
            }
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          default:
            this->text_checksum_ += data;
            *this->text_record_write_ = data;
            if (++this->text_record_write_ >= this->text_record_write_end_) {
              this->on_frame_error_(ERR_TEXTFRAME_NAME_OVERFLOW);
              this->frame_state_ = FrameState::Idle;
              goto handle_state_idle;
            }
        }
      }
      break;
    case FrameState::Value:
    handle_state_value:
      while (data_begin < data_end) {
        data = *data_begin++;
        switch (data) {
          case '\n':  // start of next record
            this->text_checksum_ += '\n';
            *this->text_record_write_ = 0;
            if (++this->text_records_count_ >= VEDIRECT_RECORDS_COUNT) {
              this->on_frame_error_(ERR_TEXTFRAME_RECORD_OVERFLOW);
              this->frame_state_ = FrameState::Idle;
              goto handle_state_idle;
            }
            this->frame_text_name_start_();
            this->frame_state_ = FrameState::Name;
            goto handle_state_name;
          case '\r':  // pre-start of next record
            this->text_checksum_ += '\r';
            break;
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          default:
            this->text_checksum_ += data;
            *this->text_record_write_ = data;
            if (++this->text_record_write_ >= this->text_record_write_end_) {
              this->on_frame_error_(ERR_TEXTFRAME_VALUE_OVERFLOW);
              this->frame_state_ = FrameState::Idle;
              goto handle_state_idle;
            }
        }
      }
      break;
    case FrameState::Idle:
    handle_state_idle:
      while (data_begin < data_end) {
        switch (*data_begin++) {
          case ':':  // HEX FRAME
            this->frame_hex_start_();
            goto handle_state_hex;
          case '\n':  // start of TEXT FRAME
            this->text_checksum_ += '\n';
            this->text_records_count_ = 0;
            this->frame_text_name_start_();
            this->frame_state_ = FrameState::Name;
            goto handle_state_name;
          case '\r':  // pre-start of TEXT FRAME
            this->text_checksum_ = '\r';
          default:
            break;
        }
      }
      break;
    case FrameState::Hex:
    handle_state_hex:
      while (data_begin < data_end) {
        switch (this->hexframe_decoder_.decode(*data_begin++)) {
          case HexFrameDecoder::Result::Continue:
            break;
          case HexFrameDecoder::Result::Valid:
            this->on_frame_hex_(this->hexframe_);
            this->frame_state_ = this->frame_state_backup_;
            goto handle_state;
          case HexFrameDecoder::Result::ChecksumError:
            this->on_frame_error_(ERR_HEXFRAME_CHECKSUM);
            this->frame_state_ = this->frame_state_backup_;
            goto handle_state;
          case HexFrameDecoder::Result::Overflow:
            this->on_frame_error_(ERR_HEXFRAME_OVERFLOW);
            this->frame_state_ = FrameState::Idle;
            goto handle_state_idle;
          default:
            // case HexFrameDecoder::Result::CodingError:
            // case HexFrameDecoder::Result::Terminated:
            this->on_frame_error_(ERR_HEXFRAME_CODING);
            this->frame_state_ = FrameState::Idle;
            goto handle_state_idle;
        }
      }
      break;
    case FrameState::Checksum:
    handle_state_checksum:
      if (data_begin < data_end) {
        if ((uint8_t) (this->text_checksum_ + *data_begin++)) {
          this->on_frame_error_(ERR_TEXTFRAME_CHECKSUM);
        } else {
          this->on_frame_text_(this->text_records_, this->text_records_count_);
        }
        this->frame_state_ = FrameState::Idle;
        goto handle_state_idle;
      }
      break;
  }
}

}  // namespace m3_vedirect
}  // namespace esphome
