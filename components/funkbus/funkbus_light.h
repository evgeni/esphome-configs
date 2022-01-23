#pragma once

#define CC1101_FREQUENCY 433.42

#include "esphome/components/light/light_output.h"
#include "esphome/core/component.h"
#include "funkbus_remote.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>

namespace esphome {
namespace funkbus {

class FunkbusLightOutput : public Component, public light::LightOutput {
public:
  FunkbusRemote *remote;
  InternalGPIOPin *emitterPin;
  uint32_t serial;
  uint8_t command;
  uint8_t group;

  void setup() override {
    this->emitterPin->pin_mode(gpio::FLAG_OUTPUT);
    this->emitterPin->digital_write(false);

    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setMHZ(CC1101_FREQUENCY);

    remote = new FunkbusRemote(emitterPin->get_pin(), serial);
  }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::ON_OFF});
    return traits;
  }

  void write_state(light::LightState *state) override {
    bool light_state;
    state->current_values_as_binary(&light_state);
    sendCommand(light_state);
  }

  void sendCommand(bool state) {
    ELECHOUSE_cc1101.SetTx();

    uint8_t action = state ? 1 : 2; // 0=STOP, 1=OFF, 2=ON, 3=SCENE
    bool longpress = false;

    remote->sendCommand(this->command, this->group, action, longpress);

    ELECHOUSE_cc1101.setSidle();
  }

  void set_pin(InternalGPIOPin *pin) { this->emitterPin = pin; }

  void set_serial(uint32_t serial) { this->serial = serial; }
  void set_switch(uint8_t command) { this->command = command; }
  void set_group(uint8_t group) { this->group = group; }
};

} // namespace funkbus
} // namespace esphome
