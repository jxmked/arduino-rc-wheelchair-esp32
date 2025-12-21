#ifndef IR_SENSOR_h
#define IR_SENSOR_h

#include <Arduino.h>

#include "./TimeInterval.h"

class IRSensor {
 public:
  IRSensor(uint16_t pin);
  void update();
  bool isObstacleDetected();
  float get_distance();

 private:
  uint16_t __pin;
  float __current_value;
  unsigned long __sum_value;  // Will be use for noise reduction
  uint8_t __sum_count;

  TimeInterval __timer;
};

#endif