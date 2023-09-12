#ifdef USE_RP2040

#include "rp2040_touch.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#include <hardware/clocks.h>
#include <hardware/gpio.h>

#include <cinttypes>

namespace esphome {
namespace rp2040_touch {

static const char *const TAG = "rp2040_touch";

static const uint32_t timeout_ticks = 10000;

void RP2040TouchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up RP2040 Touch Hub...");
  for (auto *child : this->children_) {
    gpio_disable_pulls(child->get_touch_pad());
    if (child->threshold_ > 0) continue; // Threshold defined by user
    uint32_t init_value = this->component_touch_pad_read(child->get_touch_pad());
    if(init_value >= timeout_ticks) {
        ESP_LOGCONFIG(TAG, "Touch Pad '%s' (T%" PRIu32 ") needs 1M pulldown." PRIu32, child->get_name().c_str(),
               (uint32_t) child->get_touch_pad());
    }
    child->threshold_ = (init_value*1.05) + 100;
    ESP_LOGCONFIG(TAG, "Touch Pad '%s' (T%" PRIu32 "): initial value %" PRIu32 " threshold init to %" PRIu32, child->get_name().c_str(), init_value, (uint32_t) child->get_touch_pad(), child->get_threshold());

    App.feed_wdt();
  }
}

void RP2040TouchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Config for RP2040 Touch Hub:");

  if (this->setup_mode_) {
    ESP_LOGCONFIG(TAG, "  Setup Mode ENABLED");
  }

  for (auto *child : this->children_) {
    LOG_BINARY_SENSOR("  ", "Touch Pad", child);
    ESP_LOGCONFIG(TAG, "    Pad: T%" PRIu32, (uint32_t) child->get_touch_pad());
    ESP_LOGCONFIG(TAG, "    Threshold: %" PRIu32, child->get_threshold());
  }
}

uint32_t RP2040TouchComponent::component_touch_pad_read(uint8_t tp) {
    const uint32_t n_samples = 100;
    uint16_t ticks = 0;
    for (uint16_t i = 0; i < n_samples; i++) {
        // set pad to digital output high for 10us to charge it

        gpio_set_dir(tp, GPIO_OUT);
        gpio_put(tp,false);
        delayMicroseconds(100);
        gpio_put(tp,true);
        delayMicroseconds(100);

        // set pad back to an input and take some samples
        gpio_set_dir(tp, GPIO_IN);

        while (gpio_get(tp)) {
            if (ticks >= timeout_ticks) {
                return timeout_ticks;
            }
            ticks++;
        }
    }
    return ticks;
}
    
void RP2040TouchComponent::loop() {
  const uint32_t now = millis();
  bool should_print = this->setup_mode_ && now - this->setup_mode_last_log_print_ > 250;
  for (auto *child : this->children_) {

    uint32_t value = this->component_touch_pad_read(child->get_touch_pad());
    child->set_value(value);
      
    if (should_print) {
      ESP_LOGD(TAG, "Touch Pad '%s' (T%" PRIu32 "): %" PRIu32, child->get_name().c_str(),
               (uint32_t) child->get_touch_pad(), child->get_value());
    }
      
    child->publish_state(child->get_value() > child->get_threshold());
    App.feed_wdt();
  }

  if (should_print) {
    // Avoid spamming logs
    this->setup_mode_last_log_print_ = now;
  }
}

RP2040TouchBinarySensor::RP2040TouchBinarySensor(uint8_t touch_pad, uint32_t threshold)
    : touch_pad_(touch_pad), threshold_(threshold) {}

}  // namespace rp2040_touch
}  // namespace esphome

#endif
