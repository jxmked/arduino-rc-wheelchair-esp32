#ifndef SIGNAL_LED_H
#define SIGNAL_LED_H

#include <Arduino.h>

#include "Enums.h"
#include "structs.h"

class SignalLED {
 public:
  SignalLED(SignalLEDProps props);
  void setState(E_SignalLED led, bool state);
  bool getState(E_SignalLED led);
  void update();
  void offAll();

 private:
  SignalLEDProps __props;
  SignalLEDProps __current_state;
};

#endif  // SIGNAL_LED_H