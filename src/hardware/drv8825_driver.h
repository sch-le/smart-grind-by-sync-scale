#pragma once

#include "stepper.h"

#include <Arduino.h>
#include <DRV8825.h>

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

public:
    drv8825_driver(uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint8_t reset_pin, uint8_t sleep_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us)
        : step_pin(step_pin), dir_pin(dir_pin), enable_pin(enable_pin), reset_pin(reset_pin), sleep_pin(sleep_pin) {
        drv8825_stepper.setStepsPerRotation(steps_per_rotation);
        drv8825_stepper.setStepPulseLength(step_pulse_length_us);
    }
    virtual ~drv8825_driver() = default;

    // Initialization and configuration
    bool begin() override;

    void step(uint16_t steps, step_direction direction) override;

    uint16_t get_steps() override {};

    const char* get_driver_name() const override { return "DRV8825"; }
};