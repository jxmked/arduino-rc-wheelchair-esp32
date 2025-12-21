#include "./client_connect.h"

#include <Arduino.h>

#include "./Enums.h"
#include "./structs.h"

// This is a stub implementation

ClientConnect::ClientConnect() {}

void ClientConnect::begin() {}

void ClientConnect::loop() {
  // Will always check and validate connection status here
}

bool ClientConnect::is_connected() { return true; }

void ClientConnect::connect() {}
void ClientConnect::disconnect() {}

Axis_t ClientConnect::data_received() {
  Axis_t axis_data = {0, 0};
  return axis_data;
}
