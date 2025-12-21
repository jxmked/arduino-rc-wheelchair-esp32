#include "./boot.h"

#include <Arduino.h>

#include "./TimeInterval.h"
#include "./constants.h"

// This is better here than inside of an Object
// for dynamic memory allocation reasons

byte led_step[] = {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
                   0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0,
                   0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
                   0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1};

#define BOOT_LED_COUNT 4

Boot::Boot()
    : __tick(100, 0, true),
      __leds_pin{LED_RED, LED_GREEN, LED_YELLOW, LED_BLUE},
      __step(0) {}

void Boot::begin() {
  for (int i = 0; i < BOOT_LED_COUNT; i++) {
    pinMode(__leds_pin[i], OUTPUT);
  }
}

void Boot::loop() {
  if (__tick.marked()) {
    for (uint8_t i = 0; i < BOOT_LED_COUNT; i++) {
      digitalWrite(__leds_pin[i], led_step[__step + i]);
    }

    __step++;
  }
}