#ifndef MotorController_h
#define MotorController_h

#include "Enums.h"
#include "structs.h"

class MotorController {
 public:
  MotorController(uint16_t R_PWM, uint16_t L_PWM, uint16_t EN, uint16_t R_IS,
                  uint16_t L_IS);
  MOTOR_PROPS sense_motor(uint16_t pin);
  void update();
  void disconnect();
  void stop();
  void forward();
  void reverse();
  bool is_ready();
  MotorState state();
  void kill();

 private:
  MOTOR_CONTROLLER_PINS pins;
  MotorState __current_state;
  bool __is_ready;
};

#endif
