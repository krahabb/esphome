#include "hexframe.h"
#include "entity.h"

namespace esphome {
namespace m3_vedirect {

const char hexdigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

HexFrame::HexFrame(const char *buf) {
  bool is_hinibble = false;
  uint8_t data = 0;
  uint8_t checksum = 0x55;
  for (;; ++buf) {
    uint8_t nibble = *buf;
    if (nibble < '0') {
      break;  // error or termination
    } else if (nibble <= '9') {
      nibble -= '0';
    } else if (nibble < 'A') {
      break;
    } else if (nibble <= 'F') {
      nibble -= 55;
    } else {
      break;
    }
    if (is_hinibble) {
      is_hinibble = false;
      data = nibble << 4;
    } else {
      is_hinibble = true;
      data |= nibble;
      checksum -= data;
      this->push_back(data);
    }
  }  // end for
  this->valid_ = (checksum == 0);
}

std::string HexFrame::data_hex() const {
  std::string buf;
  const_iterator data = this->cbegin() + 4;
  const_iterator data_end = this->cend() - 1;  // assume last byte is checksum
  for (; data < data_end; ++data) {
    buf += hexdigits[(*data) >> 4];
    buf += hexdigits[(*data) & 0x0F];
  }
  return buf;
}

const std::string &HexTxFrame::encode() const {
  auto &raw_frame = const_cast<std::string &>(this->raw_frame_);
  raw_frame = ":";

  bool is_hinibble = false;
  const_iterator data = this->cbegin();

  raw_frame += hexdigits[*data];  // command
  uint8_t checksum = 0x55 - *data;
  while (++data < this->cend()) {
    raw_frame += hexdigits[(*data) >> 4];
    raw_frame += hexdigits[(*data) & 0x0F];
    checksum -= *data;
  }
  raw_frame += hexdigits[checksum >> 4];
  raw_frame += hexdigits[checksum & 0x0F];
  raw_frame += '\n';
  return raw_frame;
}

}  // namespace m3_vedirect
}  // namespace esphome
