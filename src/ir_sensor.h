#ifndef IR_SENSOR_h
#define IR_SENSOR_h

#include <Arduino.h>

#include "./TimeInterval.h"

class IRSensor {
 public:
  IRSensor(int pin);
  void update();
  bool isObstacleDetected();
  float get_distance();

 private:
  int __pin;
  float __current_value;
  int __sum_value;  // Will be use for noise reduction
  int __sum_count;

  TimeInterval __timer;
};

#endif