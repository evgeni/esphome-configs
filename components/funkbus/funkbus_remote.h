/**
 * heavily inspired by the Funkbus decoder in rtl_433
 * https://github.com/merbanan/rtl_433/blob/master/src/devices/funkbus.c
 */

#pragma once

#define LONG_SYM 1000
#define SHORT_SYM 500

#define BIT_MASK(x)                                                            \
  ((((unsigned)x) >= sizeof(unsigned) * CHAR_BIT) ? (unsigned)-1               \
                                                  : (1U << (x)) - 1)

class FunkbusRemote {
private:
  byte emitterPin;
  uint32_t serial;

public:
  FunkbusRemote(byte emitterPin, uint32_t serial)
      : emitterPin(emitterPin), serial(serial) {}

  static int parity8(uint8_t byte) {
    byte ^= byte >> 4;
    byte &= 0xf;
    return (0x6996 >> byte) & 1;
  }

  static uint8_t xor_bytes(uint8_t const message[], unsigned num_bytes) {
    uint8_t result = 0;
    for (unsigned i = 0; i < num_bytes; i++) {
      result ^= message[i];
    }
    return result;
  }

  static uint8_t calc_checksum(uint8_t const *bitrow, unsigned len) {
    const uint8_t full_bytes = len / 8;
    const uint8_t bits_left = len % 8;

    uint8_t xored = xor_bytes(bitrow, full_bytes);
    if (bits_left) {
      xored ^= bitrow[full_bytes] & ~BIT_MASK(8 - bits_left);
    }

    const uint8_t xor_nibble = ((xored & 0xF0) >> 4) ^ (xored & 0x0F);

    uint8_t result = 0;
    if (xor_nibble & 0x8) {
      result ^= 0x8C;
    }
    if (xor_nibble & 0x4) {
      result ^= 0x32;
    }
    if (xor_nibble & 0x2) {
      result ^= 0xC8;
    }
    if (xor_nibble & 0x01) {
      result ^= 0x23;
    }

    result = result & 0xF;
    result |= (parity8(xored) << 4);

    return result;
  }

  static byte reverse_bits(byte bits, uint8_t len) {
    byte result = 0;
    while (len--) {
      result = (result << 1) | (bits & 1);
      bits >>= 1;
    }
    return result;
  }

  void buildFrame(byte *frame, uint32_t serial, uint8_t command, uint8_t group, uint8_t action, bool longpress, bool repeat) {
    frame[0] = reverse_bits(0x4, 4) << 4 | reverse_bits(0x3, 4);   // type 0x4 and subtype 0x3
    frame[1] = reverse_bits(serial, 8);                            // first 8 bit of serial
    frame[2] = reverse_bits(serial >> 8, 8);                       // second 8 bit of serial
    frame[3] = reverse_bits(serial >> 16, 4) << 4;                 // last 4 bit of serial, 2 bit r1, 1 bit battery, 1 bit first half of r2
    frame[4] = reverse_bits(command, 3) << 4 | reverse_bits(group, 2) << 2 | reverse_bits(action, 2) >> 1; // 1 bit for second half of r2, 3 bits for command/switch, 2 bits for group, 1 bit r3, 1 bit first half of action
    frame[5] = reverse_bits(action, 2) << 7 | repeat << 6 | longpress << 5; // 1 bit for second half of action, 1 bit for repeat, 1 bit for longpress, 1 bit parity, 4 bits checksum

    uint8_t checksum = calc_checksum(frame, 43);
    frame[5] |= (checksum & BIT_MASK(5));
  }

  void sendHigh(uint16_t durationInMicroseconds) {
    digitalWrite(emitterPin, HIGH);
    delayMicroseconds(durationInMicroseconds);
  }

  void sendLow(uint16_t durationInMicroseconds) {
    digitalWrite(emitterPin, LOW);
    delayMicroseconds(durationInMicroseconds);
  }

  void sendFrame(byte *frame) {
    byte line_state = 0;
    digitalWrite(emitterPin, LOW);

    // sync
    sendHigh(4000);
    line_state = 1;

    // Data: bits are sent one by one, starting with the MSB.
    for (byte i = 0; i < 48; i++) {
      if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 0) {
        if (line_state == 0) {
          sendHigh(LONG_SYM);
          line_state = 1;
        } else {
          sendLow(LONG_SYM);
          line_state = 0;
        }
      } else {
        if (line_state == 0) {
          sendHigh(SHORT_SYM);
          sendLow(SHORT_SYM);
          line_state = 0;
        } else {
          sendLow(SHORT_SYM);
          sendHigh(SHORT_SYM);
          line_state = 1;
        }
      }
    }

    if (line_state == 0) {
      sendHigh(SHORT_SYM);
    } else {
      sendLow(SHORT_SYM);
    }

    digitalWrite(emitterPin, LOW);
  }

  void sendCommand(uint8_t command, uint8_t group, uint8_t action,
                   bool longpress) {
    byte frame[6];
    byte repeat_frame[6];

    buildFrame(frame, serial, command, group, action, longpress, false);
    buildFrame(repeat_frame, serial, command, group, action, longpress, true);

    sendFrame(frame);
    for (int i = 0; i < 3; i++) {
      delayMicroseconds(2000);
      sendFrame(repeat_frame);
    }
  }
};
