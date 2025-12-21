#ifndef BOOT_h
#define BOOT_h

#include <Arduino.h>

#include "./TimeInterval.h"

class Boot {
 public:
  Boot();
  void begin();
  void loop();
  bool is_animating();

 private:
  TimeInterval __tick;
  uint16_t __leds_pin[4];
  uint8_t __step;
  bool __is_animating;
};

#endif  // BOOT_h