#include "esphome.h"
#include <Somfy_Remote.h>
#include <EEPROM.h>

#define EEPROM_SIZE 64

#define COVER_OPEN 1.0f
#define COVER_CLOSED 0.0f

// Array storing the multiple remotes
SomfyRemote somfyr("remote1", 0x131171); // <- Change remote name and remote code here!

class SomfyCover : public Component, public Cover {
  public:
    int stop_count;

  void setup() override {
    // need to set GPIO PIN 4 as OUTPUT, otherwise no commands will be sent
    pinMode(4, OUTPUT);
    EEPROM.begin(EEPROM_SIZE);
    stop_count = 0;
  }

  CoverTraits get_traits() override {
    auto traits = CoverTraits();
    traits.set_is_assumed_state(false);
    traits.set_supports_position(false);
    traits.set_supports_tilt(false);
    return traits;
  }

  void control(const CoverCall &call) override {
    if (call.get_position().has_value()) {
      float pos = *call.get_position();

      if (pos == COVER_OPEN) {
        ESP_LOGI("somfy", "OPEN");
        somfyr.move("UP");
      } else if (pos == COVER_CLOSED) {
        ESP_LOGI("somfy", "CLOSE");
        somfyr.move("DOWN");
      } else {
        ESP_LOGI("somfy", "WAT");
      }

      this->position = pos;
      this->publish_state();
    }

    if (call.get_stop()) {
      stop_count++;
      if (stop_count == 3) {
        ESP_LOGI("somfy", "PROG");
        somfyr.move("PROGRAM");
        stop_count = 0;
      } else {
        ESP_LOGI("somfy", "STOP");
        somfyr.move("MY");
      }
    }

    EEPROM.commit();
  }
};
