#pragma once

#include <Arduino.h>

typedef enum {
    STEP_DIRECTION_CW = 0,
    STEP_DIRECTION_CCW = 1
} step_direction;

/**
 * Abstract interface for stepper drivers.
 * 
 * Enables runtime selection between the physical stepper driver and compile-time
 * configurable mock implementations used for simulation and testing.
 * 
 * @todo: Implement a mock stepper driver for testing and simulation purposes.
 */
class StepperDriver {
public:
    virtual ~StepperDriver() = default;

    virtual bool begin() = 0;

    virtual void step(uint16_t steps, step_direction direction) = 0;

    virtual uint16_t get_steps() = 0;

    virtual const char* get_driver_name() const = 0;
};