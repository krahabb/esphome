#include "scs_bridge.h"
#include "esphome/core/log.h"
#include <Arduino.h>
#include <vector>
#include <queue>

namespace esphome {
namespace scs_bridge {

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
//static volatile uint8_t rx_buf[RX_BUFFER_SIZE];
//static volatile uint8_t rx_buf_read = 0;
//static volatile uint8_t rx_buf_write = 0;
static volatile unsigned long rx_last_micros = 0;//last received transition
static volatile unsigned long rx_next_micros = 0;//estimated start of next byte
static volatile int8_t rx_bit = -1;
static volatile bool rx_busy = false;

/*
  received data are 'framed out' based on rx timings i.e.
  every frame will end after a character timeout (RX_BYTE_TIMEOUT)
  we'll pre-alloc a suitable amount of buffers so to not mess with isr
  code overhead
*/
#define RX_FRAME_COUNT 4
struct RXFrame {
  unsigned long micros_begin;
  unsigned long micros_end;
  uint8_t data[RX_BUFFER_SIZE];
  uint8_t length;
};
static RXFrame rx_frame[RX_FRAME_COUNT];
static volatile RXFrame* rx_frame_read = rx_frame;
static volatile RXFrame* rx_frame_write = rx_frame;
static volatile RXFrame* rx_frame_end = rx_frame + RX_FRAME_COUNT;

static volatile uint8_t tx_buf[16];
static volatile uint8_t* tx_buf_read = tx_buf;
static volatile uint8_t* tx_buf_write = tx_buf;
static volatile uint8_t* tx_buf_end = tx_buf + sizeof(tx_buf) - 2;
static volatile bool tx_busy = false;
static volatile int tx_bitmask = 0;
static volatile bool tx_waitnrz = false;
static volatile int tx_repeat_count;// number of consecutive frame repeats
static volatile int tx_repeat_delay;// delay between each frame repeat


struct TXFrame {
  std::string payload;
  int         repeat_count;
};
static std::queue<struct TXFrame *> tx_queue;

/*
  Config'd pins: static because there's no way we can manage instanced
  state machines which run on 'hw constrained' interrupts ;)
*/
static uint8_t RX_PIN = 3;
static uint8_t TX_PIN = 1;

void ICACHE_RAM_ATTR rx_isr() {

  unsigned long t = micros();
  long dt = t - rx_last_micros;

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
    if (tx_buf_read != tx_buf_write) {
      //new byte to send: start bit first
      timer1_write(TX_0_TICKS);
      digitalWrite(TX_PIN, 1);
      tx_waitnrz = true;
      tx_bitmask = 1;
    } else {
      tx_busy = false;
    }
    return;
  }

  if (tx_bitmask < 256) {
    //shifting out the bits
    if (*tx_buf_read & tx_bitmask) {
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
  if (++tx_buf_read != tx_buf_write) {
    //another char in frame: STOP bit and then continue
    timer1_write(TX_STOP_TICKS);
  }
  else if (--tx_repeat_count > 0) {
    //retransmit the same frame after waiting some
    tx_buf_read = tx_buf;
    timer1_write(tx_repeat_delay);
  }
  else {
    //finished: let the bus idle for long enough to allow other transmissions
    timer1_write(TX_FRAME_IDLE_TIMEOUT);
  }
};


static const char *const TAG = "scs_bridge";
static const char *const HEX_TABLE = "0123456789ABCDEF";
static char hex_buffer[RX_BUFFER_SIZE * 2 + 1];

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

SCSBridgeComponent::SCSBridgeComponent() {

}

SCSBridgeComponent::SCSBridgeComponent(GPIOPin* rx_pin, GPIOPin* tx_pin) {
  RX_PIN = rx_pin->get_pin();
  TX_PIN = tx_pin->get_pin();
}

SCSBridgeComponent::SCSBridgeComponent(uint8_t rx_pin, uint8_t tx_pin) {
  RX_PIN = rx_pin;
  TX_PIN = tx_pin;
}

void SCSBridgeComponent::setup() {

  //register_service(&SCSBridgeComponent::send, "scs_send", { "payload", "repeat_count" });

  pinMode(RX_PIN, INPUT);
  attachInterrupt(RX_PIN, rx_isr, RISING);

  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, 0);
  timer1_isr_init();
  timer1_attachInterrupt(tx_isr);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
}

void SCSBridgeComponent::loop() {
  /*
    flush receive buffer
  */
  if (rx_busy && (micros() > rx_next_micros)) {
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

  std::string payload;
  while (rx_frame_read != rx_frame_write) {
    payload.clear();
    for (int i = 0; i < rx_frame_read->length; ++i) {
      uint8_t b = rx_frame_read->data[i];
      payload += HEX_TABLE[(b >> 4) & 0x0F];
      payload += HEX_TABLE[b & 0x0F];
    }
    ESP_LOGD(TAG, "frame received { payload: %s , micros: %lu} ", payload.c_str(), rx_frame_read->micros_begin);
    this->frame_callback_.call(payload);
    if (++rx_frame_read == rx_frame_end)
      rx_frame_read = rx_frame;
  }

  /*
    flush send_queue
  */

  if (!(tx_busy | rx_busy | tx_queue.empty())) {
    struct TXFrame *frame = tx_queue.front();
    const char *_p = frame->payload.c_str();
    ESP_LOGD(TAG, "start transmission of %s", _p);
    tx_buf_read = tx_buf_write = tx_buf;
    *tx_buf_write = 0xA8;
    uint8_t checksum = 0;
    for (;;) {
      uint8_t _hinibble, _lonibble;
      __hinibble:
        _hinibble = getnibble(*_p++);
        if (_hinibble == 0xFE)
          goto __hinibble;
        if (_hinibble == 0xFF)
          break;

      __lonibble:
        _lonibble = getnibble(*_p++);
        if (_lonibble == 0xFE)
          goto __lonibble;
        if (_lonibble == 0xFF)
          break;

      checksum ^= *++tx_buf_write = (_hinibble << 4) + _lonibble;
      if (tx_buf_write == tx_buf_end) {
        ESP_LOGD(TAG, "Warning: tx payload length too long");
        break;
      }
    }
    *++tx_buf_write = checksum;
    *++tx_buf_write = 0xA3;
    ++tx_buf_write;
    tx_repeat_count = frame->repeat_count;
    tx_repeat_delay = TX_FRAME_REPEAT_DELAY;
    tx_queue.pop();
    delete frame;
    tx_busy = true;
    timer1_write(TX_1_TICKS);//schedule the tx isr
  }
}

void SCSBridgeComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "RX pin: %d", RX_PIN);
  ESP_LOGCONFIG(TAG, "TX pin: %d", TX_PIN);
}

void SCSBridgeComponent::send(std::string payload, uint32_t repeat_count) {
  struct TXFrame* frame = new TXFrame();
  frame->payload = payload;
  frame->repeat_count = repeat_count;
  tx_queue.push(frame);
  ESP_LOGD(TAG, "scs_send : enqueuing %s", payload.c_str());
}


}  // namespace scs_bridge
}  // namespace esphome
