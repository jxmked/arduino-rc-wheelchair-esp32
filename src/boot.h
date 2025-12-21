#ifndef BOOT_h
#define BOOT_h

#include <Arduino.h>

#include "./TimeInterval.h"

class Boot {
 public:
  Boot();
  void begin();
  void loop();

 private:
  TimeInterval __tick;
  uint16_t __leds_pin[4];
  uint8_t __step;
};

#endif  // BOOT_h