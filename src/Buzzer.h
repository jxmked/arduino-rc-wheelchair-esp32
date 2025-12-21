#ifndef Buzzer_h
#define Buzzer_h

#include <Arduino.h>

class Buzzer {
 public:
  Buzzer(int buzzer_pin);
  void on();
  void off();
  void play(unsigned long start, unsigned long end);
  void stop();
  void loop();

 private:
  uint16_t __buzzer_pin;
  unsigned long __se[2];
  bool __is_playing;
  unsigned long __last_interval;
};

#endif