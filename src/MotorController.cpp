#include "MotorController.h"

#include <Arduino.h>

#include "Enums.h"
#include "TimeInterval.h"
#include "constants.h"
#include "structs.h"

MotorController::MotorController(int R_PWM, int L_PWM, int EN, int R_IS,
                                 int L_IS)
    : pins{R_PWM, L_PWM, EN, R_IS, L_IS},
      __current_state(MotorState::STOP),
      __is_ready(false) {
  pinMode(R_PWM, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(EN, OUTPUT);

#if USE_MOTOR_DRIVER
  analogSetPinAttenuation(R_IS, ADC_11db);  // 0–3.3V range
  analogSetPinAttenuation(L_IS, ADC_11db);
#endif
}

MOTOR_PROPS MotorController::sense_motor(int pin) {
  // Get motor data (Current and Voltage) from pin R_IS and L_IS
  MOTOR_PROPS values;
  int adc = analogRead(pin);                                 // 0–1023
  values.voltage = (adc * ADC_REF) / ((1 << ADC_BITS) - 1);  // convert to volts
  values.current = values.voltage * IS_RATIO;  // I_L = V_IS × 8500
  return values;
}

void MotorController::update() {
#if USE_MOTOR_DRIVER
  MOTOR_PROPS res_r = sense_motor(pins.R_IS);
  MOTOR_PROPS res_l = sense_motor(pins.L_IS);

  // Only one IS pin is valid depending on motor direction
  float max_current = max(res_r.current, res_l.current);
  float max_voltage = max(res_r.voltage, res_l.voltage);

  Serial.print("C & V: ");
  Serial.print(max_current);
  Serial.print(" : ");
  Serial.println(max_voltage);

  // Fault detection (IS voltage saturates ~3.2V)
  // We need to stop the motor as long as we get abnormal value from motor
  // sensors
  if (max_current > OVERCURRENT_LIMIT || max_voltage > FAULT_VOLTAGE) {
    __is_ready = false;
    stop();
  } else {
    __is_ready = true;
  }
#else
  // We don't have any sensing for motor status
  __is_ready = true;
#endif
}

void MotorController::stop() {
  digitalWrite(pins.EN, LOW);
  // Serial.println("Motor Stopped");

#if USE_MOTOR_DRIVER

  digitalWrite(pins.R_PWM, LOW);
  digitalWrite(pins.L_PWM, LOW);

  // we're having heat issue from the driver
  // analogWrite(pins.R_PWM, 0);
  // analogWrite(pins.L_PWM, 0);
#else
  digitalWrite(pins.R_PWM, LOW);
  digitalWrite(pins.L_PWM, LOW);
#endif

  __current_state = MotorState::STOP;
}

void MotorController::forward() {
  if (!__is_ready) return stop();
  if (__current_state == MotorState::FORWARD) return;

  digitalWrite(pins.EN, HIGH);

#if USE_MOTOR_DRIVER

  digitalWrite(pins.L_PWM, LOW);
  digitalWrite(pins.R_PWM, HIGH);

  // we're having heat issue from the driver
  // analogWrite(pins.R_PWM, MOTOR_MAX_POWER);
  // analogWrite(pins.L_PWM, 0);
#else
  digitalWrite(pins.L_PWM, LOW);
  digitalWrite(pins.R_PWM, HIGH);
#endif
  __current_state = MotorState::FORWARD;
}

void MotorController::reverse() {
  if (!__is_ready) return stop();
  if (__current_state == MotorState::REVERSE) return;

  digitalWrite(pins.EN, HIGH);

#if USE_MOTOR_DRIVER

  digitalWrite(pins.R_PWM, LOW);
  digitalWrite(pins.L_PWM, HIGH);

  // we're having heat issue from the driver
  // analogWrite(pins.R_PWM, 0);
  // analogWrite(pins.L_PWM, MOTOR_MAX_POWER);
#else
  digitalWrite(pins.R_PWM, LOW);
  digitalWrite(pins.L_PWM, HIGH);
#endif
  __current_state = MotorState::REVERSE;
}

bool MotorController::is_ready() { return __is_ready; }

MotorState MotorController::state() { return __current_state; }
