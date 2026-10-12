#include "tmc2208_driver.h"

#include <Arduino.h>
#include "../config/logging.h"
#include "../config/constants.h"

#define MICROSTEP 8u

bool tmc2208_driver::init(Preferences* preferences, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin, uint16_t steps_per_rotation, uint16_t step_pulse_length_us) {
  this->prefs = preferences;
  this->step_pin = step_pin;
  this->dir_pin = dir_pin;
  this->enable_pin = enable_pin;
  this->steps_per_rotation = steps_per_rotation * MICROSTEP;
  this->step_pulse_length_us = step_pulse_length_us;

  // Load saved rotation from non-volatile memory, default to 0;
  this->steps = prefs->getInt("tmc2208_steps", 0);  
  
  pinMode(step_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(enable_pin, OUTPUT);

  digitalWrite(enable_pin, HIGH);

  return true;
}

bool tmc2208_driver::init() {
  return false;
}

void tmc2208_driver::enable() {
  digitalWrite(enable_pin, LOW);
}

void tmc2208_driver::disable() {
  digitalWrite(enable_pin, HIGH);
}

bool tmc2208_driver::is_enabled() {
  return digitalRead(enable_pin) == LOW;
}

void tmc2208_driver::step(uint16_t steps, step_direction direction)
{
  this->direction = direction;
  int8_t dir = 0;
  
  LOG_BLE("Stepping %u steps in direction %d\n", steps, direction);

  if (is_enabled() == false) {
    digitalWrite(enable_pin, LOW);
    delayMicroseconds(GRIND_STEPPER_ENABLE_TIME_US);
  }

  switch (direction) {
    case STEP_DIRECTION_CW:
    {
      digitalWrite(dir_pin, HIGH);
      dir = -1;
      break;
    }
    case STEP_DIRECTION_CCW:
    {
      digitalWrite(dir_pin, LOW);
      dir = 1;
      break;
    }
    default:
    { return; }
  }
  
  // disable interrupts to ensure precice timing for step pulses
  noInterrupts();
  
  pinMode(step_pin, OUTPUT); /* #TODO */
  
  for (uint16_t u = 0; u < steps; u++)
  {
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(step_pulse_length_us);
    digitalWrite(step_pin, LOW);
    delayMicroseconds(step_pulse_length_us);
    this->steps += dir;
  }

  // re-enable interrupts after precise timing for step pulses
  interrupts();

  /*if (is_enabled() == true) {
    delayMicroseconds(GRIND_STEPPER_ENABLE_TIME_US);
    digitalWrite(enable_pin, HIGH);
  }*/
}

int32_t tmc2208_driver::get_steps() {
  return this->steps;
}

void tmc2208_driver::save_steps() {
  if (prefs != nullptr) {
    prefs->putInt("tmc2208_steps", this->steps);
  }
}

void tmc2208_driver::reset_steps() {
  this->steps = 0;
  save_steps();
}

float tmc2208_driver::get_rotation() {
  return (((float)this->steps * 360.0) / (float)(this->steps_per_rotation));
}