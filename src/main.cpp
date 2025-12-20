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

int led_pos = 0x1;

void setup() {
  Serial.begin(115200);

  analogReadResolution(ADC_BITS);

  Motor_1.stop();
  Motor_2.stop();
}

void loop() {
  sensor.update();

  Signal_LED.update();

  Motor_1.update();
  Motor_2.update();

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

  led_pos = led_pos << 1;

  if (led_pos > 0x10) {
    led_pos = 0x1;
  }

  Signal_LED.offAll();

  if (led_pos == 0x1) {
    Signal_LED.setState(E_SignalLED::GESTURE, true);
    buzz.off();
    Motor_1.forward();
    Motor_2.forward();
  } else if (led_pos == 0x2) {
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
  } else if (led_pos == 0x4) {
    Signal_LED.setState(E_SignalLED::LOWBAT, true);
    buzz.on();
  } else if (led_pos == 0x8) {
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);
    Motor_1.reverse();
    Motor_2.reverse();
  }

  Signal_LED.update();

  Motor_1.update();
  Motor_2.update();

  delay(500);
}
