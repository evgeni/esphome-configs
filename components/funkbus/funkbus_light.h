#pragma once

#include "esphome/components/light/light_output.h"
#include "esphome/core/component.h"
#include "funkbus_remote.h"

static const char *const TAG = "funkbus.light";

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

    remote = new FunkbusRemote(emitterPin, serial);
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
    //CC1101Component::begin_tx();

    uint8_t action = state ? 1 : 2; // 0=STOP, 1=OFF, 2=ON, 3=SCENE
    bool longpress = false;

    remote->sendCommand(this->command, this->group, action, longpress);

    //CC1101Component::set_idle();
  }

  void set_pin(InternalGPIOPin *pin) { this->emitterPin = pin; }

  void set_serial(uint32_t serial) { this->serial = serial; }
  void set_switch(uint8_t command) { this->command = command; }
  void set_group(uint8_t group) { this->group = group; }
};

} // namespace funkbus
} // namespace esphome
