#include "tmc2208_driver.h"

#include <Arduino.h>

#define MICROSTEP 1u
#define PWM_FREQ 800u

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

  // Disable stepper to prevent hold current
  digitalWrite(enable_pin, HIGH);

  return true;
}

bool tmc2208_driver::init() {
  return false;
}

void tmc2208_driver::step(uint16_t steps, step_direction direction)
{
  this->direction = direction;
  int8_t dir = 0;

  // enable stepper
  digitalWrite(enable_pin, LOW);

  switch (direction) {
    case STEP_DIRECTION_CW:
    {
      digitalWrite(dir_pin, LOW);
      dir = -1;
      break;
    }
    case STEP_DIRECTION_CCW:
    {
      digitalWrite(dir_pin, HIGH);
      dir = 1;
      break;
    }
    default:
    { return; }
  }

  for (uint16_t u = 0; u < steps; u++)
  {
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(step_pulse_length_us);
    digitalWrite(step_pin, LOW);
    delayMicroseconds(step_pulse_length_us);
    this->steps += dir;
  }
}

void tmc2208_driver::start_step(step_direction direction)
{
  if (start_time_us != 0) { return; } // already running

  this->direction = direction;

  // enable stepper
  digitalWrite(enable_pin, LOW);

  switch (direction) {
    case STEP_DIRECTION_CW:
    {
      digitalWrite(dir_pin, LOW);
      break;
    }
    case STEP_DIRECTION_CCW:
    {
      digitalWrite(dir_pin, HIGH);
      break;
    }
    default:
    { return; }
  }

  start_time_us = micros();

  analogWriteFrequency(step_pin, PWM_FREQ);
  analogWrite(step_pin, 127u);
}

void tmc2208_driver::stop_step()
{
  analogWrite(step_pin, 0u);

  if (start_time_us != 0u)
  {
    uint32_t elapsed_steps = (uint64_t)(micros() - start_time_us) * PWM_FREQ / 1000000u;
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

  // disable stepper
  digitalWrite(enable_pin, HIGH);
}

int32_t tmc2208_driver::get_steps() {
  if (start_time_us != 0)
  {
    // stepper is currently running, calculate the elapsed steps since start
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
  return (float)(this->steps) / (float)(this->steps_per_rotation);
}