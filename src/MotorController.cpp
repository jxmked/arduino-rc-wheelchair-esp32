#include "MotorController.h"

#include <Arduino.h>

#include "Enums.h"
#include "TimeInterval.h"
#include "constants.h"
#include "structs.h"

/**
 * Looking for motor power?
 * Goto:
 * "constants.h" -> MOTOR_MAX_POWER
 * */

MotorController::MotorController(uint16_t R_PWM, uint16_t L_PWM, uint16_t EN,
                                 uint16_t R_IS, uint16_t L_IS)
    : pins{R_PWM, L_PWM, EN, R_IS, L_IS},
      __current_state(MotorState::STOP),
      __is_ready(false),
      __keep_on(false),
      __power(1.0) {
  pinMode(R_PWM, OUTPUT);
  pinMode(L_PWM, OUTPUT);

  pinMode(EN, OUTPUT);

#if USE_MOTOR_DRIVER
  analogSetPinAttenuation(R_IS, ADC_11db);  // 0–3.3V range
  analogSetPinAttenuation(L_IS, ADC_11db);
#endif
}

MOTOR_PROPS MotorController::sense_motor(uint16_t pin) {
  // Get motor data (Current and Voltage) from pin R_IS and L_IS
  MOTOR_PROPS values;
  uint16_t adc = analogRead(pin);                            // 0–1023
  values.voltage = (adc * ADC_REF) / ((1 << ADC_BITS) - 1);  // convert to volts
  values.current = values.voltage * IS_RATIO;  // I_L = V_IS × 8500
  return values;
}

void MotorController::update() {
#if USE_MOTOR_DRIVER
  MOTOR_PROPS res_r = sense_motor(pins.R_IS);
  MOTOR_PROPS res_l = sense_motor(pins.L_IS);

  // Only one IS pin is valid depending on motor direction
  const float max_current = max(res_r.current, res_l.current);
  const float max_voltage = max(res_r.voltage, res_l.voltage);

  // Serial.print("C & V: ");
  // Serial.print(max_current);
  // Serial.print(" : ");
  // Serial.println(max_voltage);

  // Fault detection (IS voltage saturates ~3.2V)
  // We need to stop the motor as long as we get abnormal value from motor
  // sensors
  if (max_current > OVERCURRENT_LIMIT || max_voltage > FAULT_VOLTAGE) {
    __is_ready = false;
    stop();
  } else {
    __is_ready = true;
  }

  if (!__is_ready) return kill();
  const float use_power_f =
      constrain(__power * static_cast<float>((MOTOR_MAX_POWER)), 0, 255);
  const int use_power = static_cast<int>(use_power_f);

  switch (__current_state) {
    case MotorState::FORWARD:
      digitalWrite(pins.EN, HIGH);

#if USE_MOTOR_PWM_CHANNEL
      analogWrite(pins.R_PWM, use_power);
      analogWrite(pins.L_PWM, 0);
#else
      digitalWrite(pins.L_PWM, LOW);
      digitalWrite(pins.R_PWM, HIGH);
#endif
      break;

    case MotorState::REVERSE:
      digitalWrite(pins.EN, HIGH);

#if USE_MOTOR_PWM_CHANNEL
      analogWrite(pins.R_PWM, 0);
      analogWrite(pins.L_PWM, use_power);
#else
      digitalWrite(pins.R_PWM, LOW);
      digitalWrite(pins.L_PWM, HIGH);
#endif

      break;

    case MotorState::STOP:
      digitalWrite(pins.EN, HIGH);

      analogWrite(pins.R_PWM, 0);
      analogWrite(pins.L_PWM, 0);

      // we're having heat issue from the driver
      // analogWrite(pins.R_PWM, 0);
      // analogWrite(pins.L_PWM, 0);
      break;

    case MotorState::DISCONNECT:
      digitalWrite(pins.EN, __keep_on ? HIGH : LOW);

      analogWrite(pins.R_PWM, 0);
      analogWrite(pins.L_PWM, 0);
      break;
  }
#else
  // We don't have any sensing for motor status
  __is_ready = true;

  switch (__current_state) {
    case MotorState::FORWARD:
      digitalWrite(pins.EN, HIGH);

      digitalWrite(pins.L_PWM, LOW);
      digitalWrite(pins.R_PWM, HIGH);
      break;

    case MotorState::REVERSE:
      digitalWrite(pins.EN, HIGH);

      digitalWrite(pins.R_PWM, LOW);
      digitalWrite(pins.L_PWM, HIGH);
      break;

    case MotorState::STOP:
      digitalWrite(pins.EN, HIGH);

      digitalWrite(pins.R_PWM, LOW);
      digitalWrite(pins.L_PWM, LOW);
      break;

    case MotorState::DISCONNECT:
      digitalWrite(pins.EN, __keep_on ? HIGH : LOW);

      digitalWrite(pins.R_PWM, LOW);
      digitalWrite(pins.L_PWM, LOW);
      break;
  }
#endif

  ///////////////////

  // switch (__current_state) {
  //   case MotorState::FORWARD:
  //     Serial.println("forward");
  //     break;

  //   case MotorState::REVERSE:
  //     Serial.println("reverse");
  //     break;

  //   case MotorState::STOP:
  //     Serial.println("stop");
  //     break;

  //   case MotorState::DISCONNECT:
  //     Serial.println("disconnect");
  //     break;
  // }
}

void MotorController::disconnect(bool keep_on) {
  __current_state = MotorState::DISCONNECT;
  __keep_on = keep_on;
}

void MotorController::stop() { __current_state = MotorState::STOP; }

void MotorController::forward(float power) {
  __power = power;
  __current_state = MotorState::FORWARD;
}

void MotorController::reverse(float power) {
  __power = power;
  __current_state = MotorState::REVERSE;
}

bool MotorController::is_ready() { return __is_ready; }

MotorState MotorController::state() { return __current_state; }

void MotorController::kill() {
  digitalWrite(pins.EN, LOW);

#if USE_MOTOR_PWM_CHANNEL
  analogWrite(pins.L_PWM, 0);
  analogWrite(pins.R_PWM, 0);
#else
  digitalWrite(pins.R_PWM, LOW);
  digitalWrite(pins.L_PWM, LOW);
#endif

  disconnect(false);
}
