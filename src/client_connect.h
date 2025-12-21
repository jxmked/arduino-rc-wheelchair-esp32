#ifndef CLIENT_CONECT_h
#define CLIENT_CONECT_h

#include <Arduino.h>

#include "./Enums.h"
#include "./structs.h"

class ClientConnect {
 public:
  ClientConnect();
  void begin();
  void loop();
  bool is_connected();
  void connect();
  void disconnect();
  Axis_t data_received();

 private:
  // Add private members as needed
};

#endif  // CLIENT_CONECT_h