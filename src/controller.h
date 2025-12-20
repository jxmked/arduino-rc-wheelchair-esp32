#ifndef CONTROLLER_h
#define CONTROLLER_h

#include <Arduino.h>

#include "./Enums.h"
#include "./MotorController.h"
#include "./structs.h"

class Controller {
 public:
  Controller();

  void begin();

  void forward();
  void reverse();

  void left();
  void right();

  // Hard left and hard right is turning in place
  void hard_left();
  void hard_right();

  void stop();
  void disconnect();

  void override();

  void update();

  ControllerState state() { return __state; }

 private:
  ControllerState __state;
  MotorController M1;
  MotorController M2;
};

#endif