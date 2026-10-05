#pragma once

#include "stepper.h"

#include <Arduino.h>
#include <Preferences.h>

class tmc2208_driver : public StepperDriver {
  private:
    uint8_t enable_pin; // LOW: Driver enabled, HIGH: Driver disabled
    uint8_t step_pin;   // Step on rising edge
    uint8_t dir_pin;    // Direction: LOW: Clockwise, HIGH: Counter-clockwise

    uint16_t steps_per_rotation;    // Number of steps per full rotation
    uint16_t step_pulse_length_us;  // Length of the step pulse in microseconds

    int32_t steps;                  // Current step count
    step_direction direction;       // current direction of rotation
    Preferences* prefs = nullptr;   // Pointer to Preferences for saving step count
    uint32_t start_time_us = 0;     // Start time for step timing [µs]

  public:
  
    tmc2208_driver() = default;
    
    virtual ~tmc2208_driver() = default;

    bool init(Preferences* preferences, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us);
    
    bool init() override;

    void enable() override;

    void disable() override;

    bool is_enabled() override;

    void step(uint16_t steps, step_direction direction) override;
    
    int32_t get_steps() override;
    
    void save_steps() override;
    
    void reset_steps() override;

    float get_rotation() override;

    const char* get_driver_name() const override { return "TMC2208"; }
};