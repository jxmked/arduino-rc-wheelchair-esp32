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

  void stop();
  void disconnect();

  void update();

 private:
  ControllerState __state;

};

#endif