#pragma once

#ifdef USE_RP2040

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include <hardware/pio.h>
#include <hardware/structs/pio.h>

#include <vector>

namespace esphome {
namespace rp2040_touch {

using init_fn = void (*)(PIO pio, uint sm, uint offset, uint pin);
    
class RP2040TouchBinarySensor;

class RP2040TouchComponent : public Component {
 public:
  void register_touch_pad(RP2040TouchBinarySensor *tbs) { this->children_.push_back(tbs); }

  void set_setup_mode(bool setup_mode) { this->setup_mode_ = setup_mode; }

  uint32_t component_touch_pad_read(RP2040TouchBinarySensor* tp);

  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void set_program(const pio_program_t *program) { this->program_ = program; }
  void set_init_function(init_fn init) { this->init_pio_ = init; }

 protected:
  std::vector<RP2040TouchBinarySensor *> children_;
  bool setup_mode_{false};
  uint32_t setup_mode_last_log_print_{0};
  const pio_program_t *program_;
  init_fn init_pio_;
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
    
  void set_pio(int pio_num) { pio_num ? this->pio_ = pio1 : this->pio_ = pio0; }
  pio_hw_t* get_pio() { return this->pio_; }
  int get_sm() { return this->sm_; }
  void set_sm(int sm) { this->sm_ = sm; }
  int get_loop_max() { return this->loop_max_; }
  void update_loop_max(int l) { this->loop_max_ = l > this->loop_max_ ? l : this->loop_max_; }

 protected:
  friend RP2040TouchComponent;

  uint8_t touch_pad_{16};
  uint32_t threshold_{0};
  uint32_t value_{0};
  uint32_t loop_max_{0}; //Max number of loops achieved in PIO read
    
  pio_hw_t *pio_{nullptr};
  int sm_;
};

}  // namespace rp2040_touch
}  // namespace esphome

#endif
