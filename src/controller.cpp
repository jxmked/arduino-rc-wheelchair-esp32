#include "./controller.h"

#include <Arduino.h>

#include "./Enums.h"
#include "./MotorController.h"
#include "./constants.h"
#include "./structs.h"

Controller::Controller()
    : __state(ControllerState::IDLE),
      M1(M1_RPWM, M1_LPWM, MOTOR_ENABLE_FLAG, M1_R_IS, M1_L_IS),
      M2(M2_RPWM, M2_LPWM, MOTOR_ENABLE_FLAG, M2_R_IS, M2_L_IS) {
  __state = ControllerState::IDLE;
}

void Controller::begin() {
  M1.stop();
  M2.stop();
}

void Controller::forward() {
  M1.forward();
  M2.forward();
  __state = ControllerState::FORWARD;
}

void Controller::reverse() {
  M1.reverse();
  M2.reverse();
  __state = ControllerState::REVERSE;
}

void Controller::left() {
  M1.stop();
  M2.forward();
  __state = ControllerState::LEFT;
}
void Controller::right() {
  M2.stop();
  M1.forward();
  __state = ControllerState::RIGHT;
}

void Controller::hard_left() {
  M1.reverse();
  M2.forward();
  __state = ControllerState::HARD_LEFT;
}

void Controller::hard_right() {
  M1.forward();
  M2.reverse();
  __state = ControllerState::HARD_RIGHT;
}

void Controller::r_left() {
  M1.stop();
  M2.reverse();

  __state = ControllerState::R_LEFT;
}

void Controller::r_right() {
  M2.stop();
  M1.reverse();
  __state = ControllerState::R_RIGHT;
}

void Controller::stop() {
  M1.stop();
  M2.stop();
  __state = ControllerState::STOP;
}

void Controller::disconnect() {
  M1.disconnect();
  M2.disconnect();
  __state = ControllerState::IDLE;
}

void Controller::override() {
  stop();
  // disconnect();
  __state = ControllerState::IDLE;
}

void Controller::update() {
  M1.update();
  M2.update();

  if (!M1.is_ready() && !M2.is_ready()) {
    stop();
    M1.update();
    M2.update();
  }
}
