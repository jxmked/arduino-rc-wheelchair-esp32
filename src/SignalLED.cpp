#include "SignalLED.h"

#include <Arduino.h>

#include "Enums.h"
#include "structs.h"

SignalLED::SignalLED(SignalLEDProps props)
    : __props(props), __current_state{0, 0, 0, 0} {
  pinMode(__props.pin_gesture, OUTPUT);
  pinMode(__props.pin_override, OUTPUT);
  pinMode(__props.pin_lowbat, OUTPUT);
  pinMode(__props.pin_bt, OUTPUT);

  // Initialize all LEDs to off
  offAll();
}

void SignalLED::setState(E_SignalLED led, bool state) {
  int set_state = state ? HIGH : LOW;
  switch (led) {
    case E_SignalLED::GESTURE:
      __current_state.pin_gesture = set_state;
      break;

    case E_SignalLED::OVERRIDE:
      __current_state.pin_override = set_state;
      break;

    case E_SignalLED::LOWBAT:
      __current_state.pin_lowbat = set_state;
      break;

    case E_SignalLED::BLUETOOTH:
      __current_state.pin_bt = set_state;
      break;
  }
}

bool SignalLED::getState(E_SignalLED led) {
  switch (led) {
    case E_SignalLED::GESTURE:
      return __current_state.pin_gesture;

    case E_SignalLED::OVERRIDE:
      return __current_state.pin_override;

    case E_SignalLED::LOWBAT:
      return __current_state.pin_lowbat;

    case E_SignalLED::BLUETOOTH:
      return __current_state.pin_bt;
  }

  return false;
}

void SignalLED::update() {
  digitalWrite(__props.pin_gesture, __current_state.pin_gesture);
  digitalWrite(__props.pin_override, __current_state.pin_override);
  digitalWrite(__props.pin_lowbat, __current_state.pin_lowbat);
  digitalWrite(__props.pin_bt, __current_state.pin_bt);
}

void SignalLED::offAll() {
  setState(E_SignalLED::GESTURE, false);
  setState(E_SignalLED::OVERRIDE, false);
  setState(E_SignalLED::LOWBAT, false);
  setState(E_SignalLED::BLUETOOTH, false);
}
