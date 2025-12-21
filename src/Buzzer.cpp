#include "Buzzer.h"

#include <Arduino.h>

Buzzer::Buzzer(int buzzer_pin)
  : __buzzer_pin(buzzer_pin),
    __se{ 0, 0 },
    __is_playing(false),
    __last_interval(0) {
  pinMode(__buzzer_pin, OUTPUT);
}

void Buzzer::on() {
  digitalWrite(__buzzer_pin, HIGH);
}

void Buzzer::off() {
  digitalWrite(__buzzer_pin, LOW);
}

void Buzzer::play(unsigned long start, unsigned long end) {
  __se[0] = start;
  __se[1] = end;
  __is_playing = true;
}

void Buzzer::stop() {
  __is_playing = false;
}

void Buzzer::loop() {
  if (!__is_playing) return off();

  const unsigned long ms = millis();
  const unsigned long start = __se[0];
  const unsigned long end = __se[1];
  const unsigned long ival = __last_interval;

  const unsigned long diff = ms - ival;

  if (diff >= start) {
    if (diff - start >= end) __last_interval = ms;
    return on();
  }

  return off();
}
