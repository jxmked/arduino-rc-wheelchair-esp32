#ifndef ENUMS_h
#define ENUMS_h

enum class E_SignalLED { GESTURE, OVERRIDE, LOWBAT, BLUETOOTH };

enum class MotorState { STOP, FORWARD, REVERSE, DISCONNECT };

enum class ControllerState {
  IDLE,
  FORWARD,
  REVERSE,
  LEFT,
  RIGHT,
  STOP,
  HARD_LEFT,
  HARD_RIGHT,
  R_LEFT,
  R_RIGHT
};

enum class ClientState { DISCONNECTED, CONNECTING, CONNECTED };

#endif