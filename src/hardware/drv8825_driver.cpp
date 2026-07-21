#include "drv8825_driver.h"

#include <Arduino.h>
#include <DRV8825.h>

bool drv8825_driver::init(Preferences* preferences, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint8_t reset_pin, uint8_t sleep_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us) {
    this->prefs = preferences;
    this->step_pin = step_pin;
    this->dir_pin = dir_pin;
    this->enable_pin = enable_pin;
    this->reset_pin = reset_pin;
    this->sleep_pin = sleep_pin;
    this->steps_per_rotation = steps_per_rotation;
    this->step_pulse_length_us = step_pulse_length_us;

    drv8825_stepper.begin(dir_pin, step_pin, enable_pin, reset_pin, sleep_pin);

    // Configure steps per rotation for specific motor
    drv8825_stepper.setStepsPerRotation(steps_per_rotation);

    // Configure step pulse length for the DRV8825 driver [ms]
    drv8825_stepper.setStepPulseLength(step_pulse_length_us);

    // Load saved steps from non-volatile memory, default to 0
    drv8825_stepper.resetSteps(prefs->getUInt("drv8825_steps", 0));  

    return true;  // Initialization successful
}

bool drv8825_driver::init() {
    return false;
}

void drv8825_driver::step(uint16_t steps, step_direction direction) {
  
  drv8825_stepper.enable();

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
  drv8825_stepper.disable();
}

uint16_t drv8825_driver::get_steps() {
    return drv8825_stepper.getSteps();
}

void drv8825_driver::save_steps() {
    // Save the current step count to non-volatile memory
    prefs->putUInt("drv8825_steps", drv8825_stepper.getSteps());
}