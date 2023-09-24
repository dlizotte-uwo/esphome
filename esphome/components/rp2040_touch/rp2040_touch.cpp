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
    if(child->get_pio() == nullptr) {
        gpio_disable_pulls(child->get_touch_pad());
        if (child->get_threshold() > 0) continue; // Threshold defined by user
        uint32_t init_value = this->component_touch_pad_read(child);
        if(init_value >= timeout_ticks) {
            ESP_LOGCONFIG(TAG, "Touch Pad '%s' (T%" PRIu32 ") needs 1M pulldown." PRIu32, child->get_name().c_str(),
                   (uint32_t) child->get_touch_pad());
        }
        child->set_threshold((init_value*1.05) + 100);
        ESP_LOGCONFIG(TAG, "Touch Pad '%s' (T%" PRIu32 "): initial value %" PRIu32 " threshold init to %" PRIu32, child->get_name().c_str(), init_value, (uint32_t) child->get_touch_pad(), child->get_threshold());
    } else {
        // Using PIO
        // Load the assembled program into the PIO and get its location in the PIO's instruction memory
        uint offset = pio_add_program(child->get_pio(), this->program_);

        // Configure the state machine's PIO, and start it
        int sm = pio_claim_unused_sm(child->get_pio(), true);
        if (sm < 0) {
            ESP_LOGE(TAG, "Failed to claim PIO state machine");
            this->mark_failed();
            return;
        }
        child->set_sm(sm);
        this->init_pio_(child->get_pio(), child->get_sm(), offset, child->get_touch_pad());
        if (child->get_threshold() > 0) continue; // Threshold defined by user
        child->set_threshold(this->component_touch_pad_read(child)*1.05 + 75);
    }

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

uint32_t RP2040TouchComponent::component_touch_pad_read(RP2040TouchBinarySensor* s) {
    if(s->get_pio() == nullptr) {
        // Not using PIO - do old-fashioned way; need 1M pulldown
        int tp = s->get_touch_pad();
        const uint32_t n_samples = 100;
        uint16_t ticks = 0;
        for (uint16_t i = 0; i < n_samples; i++) {
            // set pad to digital output high for 100us to charge it

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
    } else {
        // Using PIO
        // pio_sm_clear_fifos(s->get_pio(),s->get_sm());
        uint32_t loops = 0;
        for(int reads = 0; reads < 8; reads++) {
            loops += (0xFFFFFFFF - pio_sm_get_blocking(s->get_pio(),s->get_sm()));
        }
        s->update_loop_max(loops); // Record highest number of achieved loops to set baseline
        return s->get_loop_max() - loops;
    }
}
    
void RP2040TouchComponent::loop() {
  const uint32_t now = millis();
  bool should_print = this->setup_mode_ && now - this->setup_mode_last_log_print_ > 250;
  for (auto *child : this->children_) {

    uint32_t value = this->component_touch_pad_read(child);
    child->set_value(value);
      
    if (should_print) {
      ESP_LOGD(TAG, "Touch Pad '%s' (T%" PRIu32 "): %" PRIu32 " %" PRIu32, child->get_name().c_str(),
               (uint32_t) child->get_touch_pad(), child->get_value(), child->get_loop_max());
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
