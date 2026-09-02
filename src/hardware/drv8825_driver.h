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
    step_direction direction;
    int32_t steps;
    uint32_t start_time_us = 0;  // Start time for step timing [µs]

    public:

    drv8825_driver() = default;
    
    virtual ~drv8825_driver() = default;

    bool init(Preferences* preferences, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint8_t reset_pin, uint8_t sleep_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us);
    
    bool init() override;

    void step(uint16_t steps, step_direction direction) override;

    void start_step(step_direction direction) override;

    void stop_step() override;
    
    int32_t get_steps() override;
    
    void save_steps() override;
    
    void reset_steps() override;

    float get_rotation() override;

    const char* get_driver_name() const override { return "DRV8825"; }

    private:
    //static void count_steps();
};