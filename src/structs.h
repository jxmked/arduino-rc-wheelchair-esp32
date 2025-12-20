#ifndef STRUCTS_h
#define STRUCTS_h

struct MOTOR_CONTROLLER_PINS {
  int R_PWM, L_PWM, EN, R_IS, L_IS;
};

struct MOTOR_PROPS {
  float current, voltage;
};

struct SignalLEDProps {
  int pin_gesture;
  int pin_override;
  int pin_lowbat;
  int pin_bt;
};

#endif