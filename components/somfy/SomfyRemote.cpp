#include "SomfyRemote.h"

#define SYMBOL 640

namespace esphome {
namespace somfy {

SomfyRemote::SomfyRemote(esphome::InternalGPIOPin *emitterPin, uint32_t remote, RollingCodeStorage *rollingCodeStorage)
	: emitterPin(emitterPin), remote(remote), rollingCodeStorage(rollingCodeStorage) {}

void SomfyRemote::setup() {
  this->emitterPin->pin_mode(gpio::FLAG_OUTPUT);
  this->emitterPin->digital_write(false);
}

void SomfyRemote::sendCommand(Command command, int repeat) {
	const uint16_t rollingCode = rollingCodeStorage->nextCode();
	sendCommandWithCode(command, rollingCode, repeat);
}

void SomfyRemote::sendCommandWithCode(Command command, uint16_t rollingCode, int repeat) {
	uint8_t frame[7];
	buildFrame(frame, command, rollingCode);
	sendFrame(frame, 2);
	for (int i = 0; i < repeat; i++) {
		sendFrame(frame, 7);
	}
}

void SomfyRemote::buildFrame(uint8_t *frame, Command command, uint16_t code) {
	const uint8_t button = static_cast<uint8_t>(command);
	frame[0] = 0xA7;          // Encryption key. Doesn't matter much
	frame[1] = button << 4;   // Which button did  you press? The 4 LSB will be the checksum
	frame[2] = code >> 8;     // Rolling code (big endian)
	frame[3] = code;          // Rolling code
	frame[4] = remote >> 16;  // Remote address
	frame[5] = remote >> 8;   // Remote address
	frame[6] = remote;        // Remote address

	// Checksum calculation: a XOR of all the nibbles
	uint8_t checksum = 0;
	for (uint8_t i = 0; i < 7; i++) {
		checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
	}
	checksum &= 0b1111;  // We keep the last 4 bits only

	// Checksum integration
	frame[1] |= checksum;

	// Obfuscation: a XOR of all the bytes
	for (uint8_t i = 1; i < 7; i++) {
		frame[i] ^= frame[i - 1];
	}
}

void SomfyRemote::sendFrame(uint8_t *frame, uint8_t sync) {
	if (sync == 2) {  // Only with the first frame.
		// Wake-up pulse & Silence
		sendHigh(9415);
		sendLow(9565);
    delayMicroseconds(80000);
	}

	// Hardware sync: two sync for the first frame, seven for the following ones.
	for (int i = 0; i < sync; i++) {
		sendHigh(4 * SYMBOL);
		sendLow(4 * SYMBOL);
	}

	// Software sync
	sendHigh(4550);
	sendLow(SYMBOL);

	// Data: bits are sent one by one, starting with the MSB.
	for (uint8_t i = 0; i < 56; i++) {
		if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1) {
			sendLow(SYMBOL);
			sendHigh(SYMBOL);
		} else {
			sendHigh(SYMBOL);
			sendLow(SYMBOL);
		}
	}

	// Inter-frame silence
	sendLow(415);
	delayMicroseconds(30000);
}

void SomfyRemote::sendHigh(uint16_t durationInMicroseconds) {
  emitterPin->digital_write(true);
  delayMicroseconds(durationInMicroseconds);
}

void SomfyRemote::sendLow(uint16_t durationInMicroseconds) {
  emitterPin->digital_write(false);
  delayMicroseconds(durationInMicroseconds);
}

} // namespace somfy
} // namespace esphome

