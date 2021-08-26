#include "scs_bridge.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/api/api_server.h"
#include <Arduino.h>
#include <queue>

#include "scs_cover.h"

namespace esphome {
namespace scs_bridge {

#define LOOP_LOWFREQUENCY_PERIOD 2000000 //update timed covers every so micros
/*

  Note about timings:
  9600 BPS means:
  - 1 bit lasts 1/9600 -> 104.16 microsec
  - a '0' bit is sent like: HIGH for 1/3 and LOW for 2/3 of bit duration
  - a '1' bit is sent like: LOW for the entire duration (104.16)

  so transmission of '0' should result in:
    - HIGH for 34.72 microsec
    - LOW for 69.44 microsec

  when setting the timer isr tick count we'll deal with the
  internal divided frequency (TIM_DIV16 - 5Mhz) so we'll have
  5 ticks per microsec. We should also count in the needed processing time
  since the isr is rearmed a 'few' cycles after being triggered

  Some experimental data gives:
  - sending all '1' and so using mainly TX_1_TICKS gives 3% excess duration
  - sending all '0' and so using TX_0_TICKS gives 6% excess duration

  Anyway:
  after final headaches and issues it looks like timings are not that critical
  at least on 'MARK' bits transitions (the duty cycle for '0's). I've just tried to keep the
  bit rate and the character rate (full 10 bit frame: START:8BIT:STOP) the best
  'compliant' as possible to my home scs bus.
*/

#define BIT_DURATION 104 //  microsec: 1/9600 BPS
#define BIT_TOLERANCE (BIT_DURATION / 2) //  round-up bits timing
//#define BYTE_DURATION 1042 // 10 bits->used to detect sync timout/reset since we'll never have that
#define RX_BYTE_TIMEOUT 1200 // 10 bits + some guard (scs timing is roughly 108 microsec)

//#define TX_0_TICKS 163 //34.72 microsec * 5 ticks/microsec - 6%
#define TX_0_TICKS 143 //34.72 microsec * 5 ticks/microsec - 6%
//#define TX_0RZ_TICKS 326 //69.44 microsec * 5 - 6%
#define TX_0RZ_TICKS 346 //69.44 microsec * 5 - 6%
#define TX_1_TICKS 505 //104.16 microsec * 5 - 3%
#define TX_STOP_TICKS (TX_1_TICKS + 175) // slightly longer STOP bit to meet real SCS behaviour (+35 micros)
#define TX_FRAME_REPEAT_COUNT 1 // retransmit a 'basic' frame
#define TX_FRAME_REPEAT_DELAY 16144 // delay between frame repeats (31 bits) [microsec * 5 ticks/microsec]
#define TX_FRAME_IDLE_TIMEOUT 5000 // 1 msec ? (no protocol spec available so far)

#define RX_BUFFER_SIZE 256 //use this to achieve 8 bit modulo arithmetics for indexes/pointers

/*
  Internal buffers and interrupt state machine state
*/
/*
  received data are 'framed out' based on rx timings i.e.
  every frame will end after a character timeout (RX_BYTE_TIMEOUT)
  we'll pre-alloc a suitable amount of buffers so to not mess with isr
  code overhead
*/
#define RX_FRAME_COUNT 4
struct RXFrame {
  uint32_t millis_begin;
  uint32_t micros_begin;
  uint32_t micros_end;
  uint8_t data[RX_BUFFER_SIZE];
  uint8_t length;
};
static RXFrame rx_frame[RX_FRAME_COUNT];
static volatile RXFrame* rx_frame_read = rx_frame;
static volatile RXFrame* rx_frame_write = rx_frame;
static volatile RXFrame* rx_frame_end = rx_frame + RX_FRAME_COUNT;
static volatile unsigned long rx_last_micros = 0;//last received transition
static volatile unsigned long rx_next_micros = 0;//estimated start of next byte
static volatile int8_t rx_bit = -1;
static volatile bool rx_busy = false;

struct TXFrame {
  std::vector<uint8_t>  data;
  int                   repeat;
};
static std::queue<struct TXFrame *> tx_queue;
static std::queue<struct TXFrame *> tx_cache;//keep references to TXFrames to avoid de/realloc
static TXFrame* tx_frame = NULL;//actual frame in transmission
static uint8_t* tx_ptr = NULL;//actual byte to send: starts from tx_frame->data.begin()
static uint8_t* tx_end = NULL;//set to actual tx_frame->data.end()
static int tx_bitmask = 0;
static bool tx_waitnrz = false;

/*
  Config'd pins: static because there's no way (or I'm lazy) we can manage instanced
  state machines which run on 'hw constrained' interrupts ;)
*/
static uint8_t RX_PIN = 3;
static uint8_t TX_PIN = 1;

void ICACHE_RAM_ATTR rx_isr() {

  uint32_t t = micros();
  uint32_t dt = t - rx_last_micros;

  if (dt > RX_BYTE_TIMEOUT) {
    rx_next_micros = t + RX_BYTE_TIMEOUT;
    if (rx_bit != -1) {
      rx_frame_write->length += 1;
      rx_frame_write->micros_end = rx_last_micros;
      rx_bit = -1;
    }
    if (rx_frame_write->length) {
      if (++rx_frame_write == rx_frame_end)
        rx_frame_write = rx_frame;
      rx_frame_write->length = 0;
    }
    rx_frame_write->millis_begin = millis();
    rx_frame_write->micros_begin = t;
    rx_frame_write->data[0] = 0xFF;
    rx_busy = true;
  } else if (dt <= BIT_TOLERANCE) {
    //spurious spike ?!
  } else {
    rx_bit += (dt + BIT_TOLERANCE) / BIT_DURATION;
    if (rx_bit < 8) {
      rx_frame_write->data[rx_frame_write->length] ^= (1 << rx_bit);
    } else {
      rx_next_micros = t + RX_BYTE_TIMEOUT;
      rx_frame_write->length += 1;
      rx_frame_write->data[rx_frame_write->length] = 0xFF;
      rx_bit = -1;
    }
  }
  rx_last_micros = t;
};

void ICACHE_RAM_ATTR tx_isr() {

  if (tx_waitnrz) {
    timer1_write(TX_0RZ_TICKS);
    digitalWrite(TX_PIN, 0);
    tx_waitnrz = false;
    return;
  }

  if (tx_bitmask == 0) {
    //state machine is idle
    if (tx_ptr != tx_end) {
      //new byte to send: start bit first
      timer1_write(TX_0_TICKS);
      digitalWrite(TX_PIN, 1);
      tx_waitnrz = true;
      tx_bitmask = 1;
    } else {
      tx_cache.push(tx_frame);
      tx_frame = NULL;
    }
    return;
  }

  if (tx_bitmask < 256) {
    //shifting out the bits
    if (*tx_ptr & tx_bitmask) {
      //'1' bit to send: transmit 'low' on the bus for 104 microsec
      timer1_write(TX_1_TICKS);
      digitalWrite(TX_PIN, 0);
    } else {
      //'0' bit to send: transmit 'hi' on the bus for 35 microsec
      timer1_write(TX_0_TICKS);
      digitalWrite(TX_PIN, 1);
      tx_waitnrz = true;
    }
    tx_bitmask <<= 1;
    return;
  }

  //set the bus to idle and (eventually) reschedule
  digitalWrite(TX_PIN, 0);
  tx_bitmask = 0;
  if (++tx_ptr != tx_end) {
    //another char in frame: STOP bit and then continue
    timer1_write(TX_STOP_TICKS);
  }
  else if (--tx_frame->repeat > 0) {
    //retransmit the same frame after waiting some
    tx_ptr = &(*tx_frame->data.begin());
    timer1_write(TX_FRAME_REPEAT_DELAY);
  }
  else {
    //finished: let the bus idle for long enough to allow other transmissions
    timer1_write(TX_FRAME_IDLE_TIMEOUT);
  }
};


//static const char *const TAG = "scs_bridge";
static const char *const HEX_TABLE = "0123456789ABCDEF";


uint8_t getnibble(const char c) {
  if (c < ' ')
    return 0xFF; //terminate
  if (c < '0')
    return 0xFE; //skip
  if (c <= '9')
    return c - '0';
  if (c < 'A')
    return 0xFE; //skip
  if (c <= 'F')
    return c - 55;
  if (c < 'a')
    return 0xFE; //skip
  if (c <= 'f')
    return c - 87;
  return 0xFE; //skip
};

const char *const SCSBridge::TAG = "scs_bridge";
SCSBridge *SCSBridge::instance_;
std::vector<SCSCover *> SCSBridge::covers_;

SCSBridge::SCSBridge() { SCSBridge::instance_ = this; }

SCSBridge::SCSBridge(uint8_t rx_pin, uint8_t tx_pin, std::string cover_name_template)
  : cover_name_template(cover_name_template) {
  SCSBridge::instance_ = this;
  RX_PIN = rx_pin;
  TX_PIN = tx_pin;
}

void SCSBridge::setup() {

  pinMode(RX_PIN, INPUT);
  attachInterrupt(RX_PIN, rx_isr, RISING);

  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, 0);
  timer1_isr_init();
  timer1_attachInterrupt(tx_isr);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);

  //query all physical devices
  SCSBridge::send(SCS_ADR_BROADCAST_QUERY, SCS_ADR_SCSBRIDGE, SCS_CMD_GET, 0x00);
}

void SCSBridge::loop() {
  /*
    flush receive buffer
  */
  uint32_t current_micros = micros();
  if (rx_busy && (current_micros > rx_next_micros)) {
    ETS_INTR_LOCK();
    rx_busy = false;
    if (rx_bit != -1) {
      rx_frame_write->length += 1;
      rx_bit = -1;
    }
    rx_frame_write->micros_end = rx_last_micros;
    if (++rx_frame_write == rx_frame_end)
      rx_frame_write = rx_frame;
    rx_frame_write->length = 0;
    ETS_INTR_UNLOCK();
  }

  std::string framestr;
  while (rx_frame_read != rx_frame_write) {
    framestr.clear();
    for (int i = 0; i < rx_frame_read->length; ++i) {
      uint8_t b = rx_frame_read->data[i];
      framestr += HEX_TABLE[(b >> 4) & 0x0F];
      framestr += HEX_TABLE[b & 0x0F];
    }
    ESP_LOGD(TAG, "frame received { frame: %s , micros: %lu} ", framestr.c_str(), rx_frame_read->micros_begin);

    if ((rx_frame_read->length >= 7) &&
      (rx_frame_read->data[0] == 0xA8) &&
      (rx_frame_read->data[6] == 0xA3)) {
      uint8_t dst_address = rx_frame_read->data[1];
      uint8_t src_address = rx_frame_read->data[2];
      uint8_t command = rx_frame_read->data[3];
      uint8_t value = rx_frame_read->data[4];
      if ((dst_address ^ src_address ^ command ^ value) == rx_frame_read->data[5]) {
        switch (dst_address) {
          case SCS_ADR_BROADCAST_STATUS://broadcast address -> src_address publishing status
            switch (command) {
              case SCS_CMD_SET:
                switch (value) {
                  case SCS_VAL_COVER_UP:
                    getcover_(src_address)->command_up(rx_frame_read->millis_begin);
                    break;
                  case SCS_VAL_COVER_DOWN:
                    getcover_(src_address)->command_down(rx_frame_read->millis_begin);
                    break;
                  case SCS_VAL_COVER_STOP:
                    getcover_(src_address)->command_stop(rx_frame_read->millis_begin);
                    break;
                }
            }
            break;// case SCS_ADR_BROADCAST
        }
      }
    }

    if (++rx_frame_read == rx_frame_end)
      rx_frame_read = rx_frame;

    this->frame_callback_.call(framestr);
  }

  /*
    flush send_queue
  */

  if (!((tx_frame != NULL) | rx_busy | tx_queue.empty())) {
    tx_frame = tx_queue.front();
    tx_queue.pop();
    framestr.clear();
    for (uint8_t b : tx_frame->data) {
      framestr += HEX_TABLE[(b >> 4) & 0x0F];
      framestr += HEX_TABLE[b & 0x0F];
    }
    ESP_LOGD(TAG, "sending frame { frame: %s , micros: %lu} ", framestr.c_str(), current_micros);
    tx_ptr = &(*tx_frame->data.begin());
    tx_end = &(*tx_frame->data.end());
    timer1_write(TX_1_TICKS);  // schedule the tx isr
  }

}

void SCSBridge::dump_config() {
  ESP_LOGCONFIG(TAG, "RX pin: %d", RX_PIN);
  ESP_LOGCONFIG(TAG, "TX pin: %d", TX_PIN);
}

/*static*/ void SCSBridge::send(std::vector<uint8_t> payload, uint32_t repeat) {
  struct TXFrame *frame;
  if (tx_cache.empty()) {
    frame = new TXFrame();
  } else {
    frame = tx_cache.front();
    tx_cache.pop();
  }
  frame->data.clear();
  frame->data.push_back(0xA8);
  uint8_t checksum = 0;
  for (std::vector<uint8_t>::iterator p = payload.begin(); p < payload.end(); ++p) {
    checksum ^= *p;
    frame->data.push_back(*p);
  }
  frame->data.push_back(checksum);
  frame->data.push_back(0xA3);
  frame->repeat = repeat;
  tx_queue.push(frame);
  //ESP_LOGD(TAG, "scs_send : enqueuing %s", payload.c_str());
}

/*static*/ void SCSBridge::send(std::string payload, uint32_t repeat) {

  std::vector<uint8_t> _data;
  for (const char* p = payload.c_str();;) {
    uint8_t _hinibble, _lonibble;
    __hinibble:
      _hinibble = getnibble(*p++);
      if (_hinibble == 0xFE)
        goto __hinibble;
      if (_hinibble == 0xFF)
        break;

    __lonibble:
      _lonibble = getnibble(*p++);
      if (_lonibble == 0xFE)
        goto __lonibble;
      if (_lonibble == 0xFF)
        break;

    _data.push_back((_hinibble << 4) + _lonibble);
  }
  SCSBridge::send(_data, repeat);
}

/*static*/ void SCSBridge::send(uint8_t dst_address, uint8_t src_address, uint8_t command, uint8_t value) {
  struct TXFrame *frame;
  if (tx_cache.empty()) {
    frame = new TXFrame();
  } else {
    frame = tx_cache.front();
    tx_cache.pop();
  }
  frame->data.clear();
  frame->data.push_back(0xA8);
  frame->data.push_back(dst_address);
  frame->data.push_back(src_address);
  frame->data.push_back(command);
  frame->data.push_back(value);
  frame->data.push_back(dst_address^src_address^command^value);
  frame->data.push_back(0xA3);
  frame->repeat = 1;
  tx_queue.push(frame);
}

/*static*/ void SCSBridge::register_cover(SCSCover *cover) {
  SCSBridge::covers_.push_back(cover);
  App.register_component(cover);
}

SCSCover *SCSBridge::getcover_(uint8_t address) {
  for (auto cover : SCSBridge::covers_) {
    if (cover->address == address)
      return cover;
  }

  /*
    dynamically adding a component/nameable to core:
    this could be tricky. Right now I see
    the component at this stage will not be added to the looping
    components but that's no issue so far since
    scs components rely on the Scheduler to process execution
  */
  SCSCover *cover = new SCSCover(address,
    this->cover_name_template + HEX_TABLE[(address >> 4) & 0x0F] + HEX_TABLE[address & 0x0F]);

  App.register_cover(cover);
  /*
    at this stage api::global_api_server has already looped through registered entities
    so we'll 'manually' add this cover
  */
  if (api::global_api_server)
    cover->add_on_state_callback([cover](){ api::global_api_server->on_cover_update(cover); });

  return cover;
}

}  // namespace scs_bridge
}  // namespace esphome
