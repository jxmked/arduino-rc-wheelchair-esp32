#include <Arduino.h>

#include "./Buzzer.h"
#include "./Enums.h"
#include "./MotorController.h"
#include "./SignalLED.h"
#include "./TimeInterval.h"
#include "./constants.h"
#include "./ir_sensor.h"
#include "./structs.h"

SignalLED Signal_LED({
    .pin_gesture = LED_BLUE,
    .pin_override = LED_RED,
    .pin_lowbat = LED_YELLOW,
    .pin_bt = LED_GREEN,
});

Buzzer buzz(BUZZER_PIN);

MotorController Motor_1(M1_RPWM, M1_LPWM, MOTOR_ENABLE_FLAG, M1_R_IS, M1_L_IS);
MotorController Motor_2(M2_RPWM, M2_LPWM, MOTOR_ENABLE_FLAG, M2_R_IS, M2_L_IS);

IRSensor sensor(IR_SENSOR_INPUT);

//******* For Testing *******//
#define LEFT_RIGHT_PIN 14
#define FRONT_BACK_PIN 27

////////////////////////////////

int led_pos = 0x1;

void setup() {
  Serial.begin(115200);

  analogReadResolution(ADC_BITS);

  Motor_1.stop();
  Motor_2.stop();
}

void loop() {
  // Priority to obstacle detection
  sensor.update();

  if (sensor.isObstacleDetected()) {
    Motor_1.stop();
    Motor_2.stop();

    Signal_LED.setState(E_SignalLED::GESTURE, true);
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    Signal_LED.setState(E_SignalLED::LOWBAT, true);
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);

    Serial.println("Obstacle Detected! Stopping Motors.");

    return;
  }

  const uint16_t left_right_value = analogRead(LEFT_RIGHT_PIN);
  const uint16_t front_back_value = analogRead(FRONT_BACK_PIN);

  Serial.print("LR & FB Values: ");
  Serial.print(left_right_value);
  Serial.print(" , ");
  Serial.println(front_back_value);
  delay(50);

  return;

  Signal_LED.update();

  Motor_1.update();
  Motor_2.update();
}
