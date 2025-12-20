#include "Buzzer.h"

#include <Arduino.h>

Buzzer::Buzzer(int buzzer_pin)
  : __buzzer_pin(buzzer_pin) {
  pinMode(__buzzer_pin, OUTPUT);
}

void Buzzer::on() {
  digitalWrite(__buzzer_pin, HIGH);
}

void Buzzer::off() {
  digitalWrite(__buzzer_pin, LOW);
}
