#pragma once

#ifdef USE_RP2040

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include <vector>

namespace esphome {
namespace rp2040_touch {

class RP2040TouchBinarySensor;

class RP2040TouchComponent : public Component {
 public:
  void register_touch_pad(RP2040TouchBinarySensor *tbs) { this->children_.push_back(tbs); }

  void set_setup_mode(bool setup_mode) { this->setup_mode_ = setup_mode; }

  uint32_t component_touch_pad_read(uint8_t tp);

  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  std::vector<RP2040TouchBinarySensor *> children_;
  bool setup_mode_{false};
  uint32_t setup_mode_last_log_print_{0};
  // common parameters
};

/// Simple helper class to expose a touch pad value as a binary sensor.
class RP2040TouchBinarySensor : public binary_sensor::BinarySensor {
 public:
  RP2040TouchBinarySensor(uint8_t touch_pad, uint32_t threshold);

  uint8_t get_touch_pad() const { return this->touch_pad_; }
  uint32_t get_threshold() const { return this->threshold_; }
  void set_threshold(uint32_t threshold) { this->threshold_ = threshold; }
  uint32_t get_value() const { return this->value_; }
  void set_value(uint32_t value) { this->value_ = value; }

 protected:
  friend RP2040TouchComponent;

  uint8_t touch_pad_{16};
  uint32_t threshold_{0};
  uint32_t value_{0};
};

}  // namespace rp2040_touch
}  // namespace esphome

#endif
