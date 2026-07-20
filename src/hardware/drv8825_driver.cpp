#include "drv8825_driver.h"

#include <Arduino.h>
#include <DRV8825.h>

drv8825_driver::drv8825_driver(uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint8_t reset_pin, uint8_t sleep_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us)
    : step_pin(step_pin), dir_pin(dir_pin), enable_pin(enable_pin), reset_pin(reset_pin), sleep_pin(sleep_pin), steps_per_rotation(steps_per_rotation), step_pulse_length_us(step_pulse_length_us) {
}

bool drv8825_driver::begin() {
    drv8825_stepper.begin(dir_pin, step_pin, enable_pin, reset_pin, sleep_pin);

    // Configure steps per rotation for specific motor
    drv8825_stepper.setStepsPerRotation(steps_per_rotation);

    // Configure step pulse length for the DRV8825 driver [ms]
    drv8825_stepper.setStepPulseLength(step_pulse_length_us);

    return true;  // Initialization successful
}

void drv8825_driver::step(uint16_t steps, step_direction direction) {
  
  switch(direction) {
    case STEP_DIRECTION_CW:
      drv8825_stepper.setDirection(DRV8825_CLOCK_WISE);
      break;
    case STEP_DIRECTION_CCW:
      drv8825_stepper.setDirection(DRV8825_COUNTERCLOCK_WISE);
      break;
    default:
      // Invalid direction, default to clockwise
      drv8825_stepper.setDirection(DRV8825_CLOCK_WISE);
      break;
  }
  
  for (uint16_t u = 0; u < steps; ++u) {
    drv8825_stepper.step();
  }
}

uint16_t drv8825_driver::get_steps() {
    return drv8825_stepper.getSteps();
}
