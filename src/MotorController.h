#ifndef MotorController_h
#define MotorController_h

#include "Enums.h"
#include "structs.h"

class MotorController {
 public:
  MotorController(int R_PWM, int L_PWM, int EN, int R_IS, int L_IS);
  MOTOR_PROPS sense_motor(int pin);
  void update();
  void disconnect();
  void stop();
  void forward();
  void reverse();
  bool is_ready();
  MotorState state();

 private:
  MOTOR_CONTROLLER_PINS pins;
  MotorState __current_state;
  bool __is_ready;
};

#endif
