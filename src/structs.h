#ifndef STRUCTS_h
#define STRUCTS_h

struct MOTOR_CONTROLLER_PINS {
  uint16_t R_PWM, L_PWM, EN, R_IS, L_IS;
};

struct MOTOR_PROPS {
  float current, voltage;
};

struct SignalLEDProps {
  uint16_t pin_gesture, pin_override, pin_lowbat, pin_bt;
};

#endif