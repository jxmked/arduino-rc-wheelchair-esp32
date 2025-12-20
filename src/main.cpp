#include <Arduino.h>

#include "./Buzzer.h"
#include "./Enums.h"
#include "./MotorController.h"
#include "./SignalLED.h"
#include "./TimeInterval.h"
#include "./constants.h"
#include "./structs.h"

SignalLED Signal_LED({
    .pin_gesture = LED_BLUE,
    .pin_override = LED_RED,
    .pin_lowbat = LED_YELLOW,
    .pin_bt = LED_GREEN,
});

MotorController Motor_1(34, 32, 4, 39, 36);
MotorController Motor_2(26, 25, 4, 35, 34);

int led_pos = 0x1;

void setup() {
  Serial.begin(115200);

  analogReadResolution(ADC_BITS);

  Motor_1.stop();
  Motor_2.stop();
}

void loop() {
  // Motor_1.update();
  // Motor_2.update();

  led_pos = led_pos << 1;

  if (led_pos > 0x10) {
    led_pos = 0x1;
  }

  Signal_LED.offAll();

  if (led_pos == 0x1) {
    Signal_LED.setState(E_SignalLED::GESTURE, true);
  } else if (led_pos == 0x2) {
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
  } else if (led_pos == 0x4) {
    Signal_LED.setState(E_SignalLED::LOWBAT, true);
  } else if (led_pos == 0x8) {
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);
  }

  Signal_LED.update();
  delay(500);
}
