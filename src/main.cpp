#include <Arduino.h>

#include "./Button.h"
#include "./Buzzer.h"
#include "./Enums.h"
#include "./MotorController.h"
#include "./SignalLED.h"
#include "./TimeInterval.h"
#include "./constants.h"
#include "./controller.h"
#include "./ir_sensor.h"
#include "./structs.h"

SignalLED Signal_LED({
    .pin_gesture = LED_BLUE,
    .pin_override = LED_RED,
    .pin_lowbat = LED_YELLOW,
    .pin_bt = LED_GREEN,
});

Buzzer buzz(BUZZER_PIN);

IRSensor sensor(IR_SENSOR_INPUT);

Controller mc;

Button btn_override(OVERRIDE_PUSH_BTN);

//******* For Testing *******//
#define LEFT_RIGHT_PIN 14
#define FRONT_BACK_PIN 27

////////////////////////////////

TimeInterval deoverride_timer(5000, 0, true);
bool is_override = false;

void setup() {
  Serial.begin(115200);

  analogReadResolution(ADC_BITS);

  btn_override.begin();
  deoverride_timer.pause();
  mc.begin();
}

void loop() {
  // Priority to obstacle detection
  sensor.update();
  mc.update();

  Signal_LED.update();

  if (btn_override.pressed()) {
    Signal_LED.offAll();
    mc.override();

    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    Serial.println("Override Button Pressed! Motors Disconnected.");

    deoverride_timer.reset();
    deoverride_timer.resume();
    is_override = true;

    return;
  }

  if (is_override) {
    if (deoverride_timer.marked()) {
      is_override = false;
      deoverride_timer.pause();
      Signal_LED.setState(E_SignalLED::OVERRIDE, false);
      Serial.println("Override Period Ended. Motors Re-Enabled.");
    }

    return;
  }

  if (sensor.isObstacleDetected()) {
    mc.stop();

    Signal_LED.setState(E_SignalLED::GESTURE, true);
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    Signal_LED.setState(E_SignalLED::LOWBAT, true);
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);

    Serial.println("Obstacle Detected! Stopping Motors.");

    return;
  } else {
    Signal_LED.offAll();
  }

  //******* For Testing *******//

  const uint16_t left_right_value = analogRead(LEFT_RIGHT_PIN);
  const uint16_t front_back_value = analogRead(FRONT_BACK_PIN);
  const int __y = (int)left_right_value - 512;
  const int __x = (int)front_back_value - 512;

  int x = 0;
  int y = 0;

  if (__x <= -510) {
    x = -1;
  } else if (__x >= 510) {
    x = 1;
  }

  if (__y <= -510) {
    y = -1;
  } else if (__y >= 510) {
    y = 1;
  }

  if (y == 1) {
    if (x == 1) {
      mc.right();
    } else if (x == -1) {
      mc.left();
    } else {
      mc.hard_right();
    }
  } else if (y == -1) {
    if (x == -1) {
      mc.right();
    } else if (x == 1) {
      mc.left();
    } else {
      mc.hard_left();
    }
  } else {
    if (x == 1) {
      mc.forward();
    } else if (x == -1) {
      mc.reverse();
    }
  }

  if (y == 0 && x == 0) {
    mc.stop();
  }

  if (mc.state() != ControllerState::IDLE &&
      mc.state() != ControllerState::STOP) {
    Signal_LED.setState(E_SignalLED::GESTURE, true);
  }
}
