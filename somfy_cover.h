#include "esphome.h"
#include <NVSRollingCodeStorage.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <SomfyRemote.h>

#define EMITTER_GPIO 2
#define REMOTE 0x5184c8

#define CC1101_FREQUENCY 433.42

#define COVER_OPEN 1.0f
#define COVER_CLOSED 0.0f

NVSRollingCodeStorage rollingCodeStorage("somfy", "badezimmer");
SomfyRemote somfyRemote(EMITTER_GPIO, REMOTE, &rollingCodeStorage);

class SomfyCover : public Component, public Cover {
public:
  void setup() override {
    somfyRemote.setup();

    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setMHZ(CC1101_FREQUENCY);
  }

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(true);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    return traits;
  }

  void sendCC1101Command(Command command) {
    ELECHOUSE_cc1101.SetTx();
    somfyRemote.sendCommand(command);
    ELECHOUSE_cc1101.setSidle();
  }

  void control(const CoverCall &call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI("somfy", "OPEN");
        sendCC1101Command(Command::Up);
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI("somfy", "CLOSE");
        sendCC1101Command(Command::Down);
      } else {
        ESP_LOGI("somfy", "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      ESP_LOGI("somfy", "STOP");
      sendCC1101Command(Command::My);
    }
  }

  void program() {
    ESP_LOGI("somfy", "PROG");
    sendCC1101Command(Command::Prog);
  }
};
