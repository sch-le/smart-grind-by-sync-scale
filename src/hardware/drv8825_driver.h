#pragma once

#include "stepper.h"

#include <Arduino.h>
#include <DRV8825.h>
#include <Preferences.h>

class drv8825_driver : public StepperDriver {
  private:
    uint8_t step_pin;
    uint8_t dir_pin;
    uint8_t enable_pin;
    uint8_t reset_pin;
    uint8_t sleep_pin;
    uint16_t steps_per_rotation;
    uint16_t step_pulse_length_us;
    DRV8825 drv8825_stepper;
    Preferences* prefs = nullptr;  // Pointer to Preferences for saving step count

  public:
    drv8825_driver() = default;
    
    virtual ~drv8825_driver() = default;

    bool init(Preferences* preferences, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint8_t reset_pin, uint8_t sleep_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us);
    
    bool init() override;

    void step(uint16_t steps, step_direction direction) override;

    uint16_t get_steps() override;

    void save_steps() override;

    const char* get_driver_name() const override { return "DRV8825"; }
};