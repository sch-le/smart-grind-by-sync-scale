#include "drv8825_driver.h"

#include <Arduino.h>
#include <DRV8825.h>
#include "bluetooth/manager.h"

#define MICROSTEP 32u
#define PWM_FREQ 800u

bool drv8825_driver::init(Preferences* preferences, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint8_t reset_pin, uint8_t sleep_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us) {
  this->prefs = preferences;
  this->step_pin = step_pin;
  this->dir_pin = dir_pin;
  this->enable_pin = enable_pin;
  this->reset_pin = reset_pin;
  this->sleep_pin = sleep_pin;
  this->steps_per_rotation = steps_per_rotation * MICROSTEP;
  this->step_pulse_length_us = step_pulse_length_us;

  // Initialize DRV8825 stepper library
  (void)drv8825_stepper.begin(dir_pin, step_pin, enable_pin, reset_pin, sleep_pin);
  drv8825_stepper.setStepPulseLength(step_pulse_length_us);
  
  // Load saved rotation from non-volatile memory, default to 0;
  this->steps = prefs->getInt("drv8825_steps", 0);  
  
  // Disable stepper to prevent hold current
  drv8825_stepper.disable();

  return true;
}

bool drv8825_driver::init() {
  return false;
}

void drv8825_driver::step(uint16_t steps, step_direction direction) {
  this->direction = direction;
  
  drv8825_stepper.enable();

  switch (direction) {
    case STEP_DIRECTION_CW:
    {
      drv8825_stepper.setDirection(DRV8825_CLOCK_WISE);
      this->steps -= steps;
      break;
    }
    case STEP_DIRECTION_CCW:
    {
      drv8825_stepper.setDirection(DRV8825_COUNTERCLOCK_WISE);
      this->steps += steps;
      break;
    }
    default:
    { return; }
  }
  
  delay(5);  // #TODO: Add delay from datasheet
  
  for (uint16_t u = 0; u < steps; u++) {
    drv8825_stepper.step();
  }
  
  drv8825_stepper.disable();
}

void drv8825_driver::start_step(step_direction direction)
{
 if (drv8825_stepper.isEnabled()) { return; }

 this->direction = direction;
 
 drv8825_stepper.enable();
 switch (direction) {
    case STEP_DIRECTION_CW:
    {
      drv8825_stepper.setDirection(DRV8825_CLOCK_WISE);
      break;
    }
    case STEP_DIRECTION_CCW:
    {
      drv8825_stepper.setDirection(DRV8825_COUNTERCLOCK_WISE);
      break;
    }
    default:
    { break; }
  }
  
  start_time_us = micros();

  analogWriteFrequency(step_pin, PWM_FREQ);
  analogWrite(step_pin, 127u);
}

void drv8825_driver::stop_step(void)
{
  analogWrite(step_pin, 0u);

  if (start_time_us != 0u)
  {
    uint32_t elapsed_steps = (uint64_t)(micros()  - start_time_us) * PWM_FREQ / 1000000u;
    start_time_us = 0u;
    
    switch (direction) {
      case STEP_DIRECTION_CW:
      {
        this->steps -= elapsed_steps;
        break;
      }
      case STEP_DIRECTION_CCW:
      {
        this->steps += elapsed_steps;
        break;
      }
      default:
      { break; }
    }
  }

  drv8825_stepper.disable();
}

int32_t drv8825_driver::get_steps() {
  if (start_time_us != 0)
  {
    // steps is currently running, calculate the elapsed steps since start
    uint32_t new_start_time_us = micros();
    uint32_t elapsed_steps = (uint64_t)(new_start_time_us - start_time_us) * PWM_FREQ / 1000000u;
    start_time_us = new_start_time_us;
    
    switch (direction) {
      case STEP_DIRECTION_CW:
      {
        this->steps -= elapsed_steps;
        break;
      }
      case STEP_DIRECTION_CCW:
      {
        this->steps += elapsed_steps;
        break;
      }
      default:
      { break; }
    }
  }

  return this->steps;
}

void drv8825_driver::save_steps() {
  // Save the current rotation to non-volatile memory
  prefs->putInt("drv8825_steps", this->steps);
}

void drv8825_driver::reset_steps() {
    this->steps = 0;
    save_steps();
}

float drv8825_driver::get_rotation() {
  return (this->get_steps() * 360.0) / (float)this->steps_per_rotation ;
}