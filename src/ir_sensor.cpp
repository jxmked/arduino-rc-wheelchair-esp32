#include "./ir_sensor.h"

#include <Arduino.h>

#include "./TimeInterval.h"
#include "./constants.h"
#include "./structs.h"

/**
 * Most code here is for testing purpose only
 *
 * Will modify after integrating with main project
 */

IRSensor::IRSensor(int pin)
    : __pin(pin), __current_value(0.0), __sum_count(0), __timer(1, 0, true) {}

void IRSensor::update() {
  // This way, we can reduce the noise by averaging multiple samples
  // NOTE: KEEP THIS FUNCTION CALL FREQUENTLY IN THE MAIN LOOP
  if (__timer.marked()) {
    uint16_t value = analogRead(__pin);
    // value = map(value, 0, 1024, 0, 4095);  // just to be sure
    __sum_value += value;
    __sum_count++;
  }

  if (__sum_count >= SAMPLING_SIZE) {
    __current_value =
        static_cast<float>(__sum_value) / static_cast<float>(SAMPLING_SIZE);

    __sum_value = 0;
    __sum_count = 0;
  }

  // Never make the __current value zero to avoid division by zero errors
  // __current_value = constrain(__current_value, 1.0f, 4095.0);
}

bool IRSensor::isObstacleDetected() {
  float distance = get_distance();

  if (distance <= static_cast<float>(OBSTACLE_DISTANCE_THRESHOLD)) {
    return true;
  }

  return false;
}

float IRSensor::get_distance() {
  float voltage = static_cast<float>(__current_value);

  float distance = map(voltage, 0, 1023, 20, 150);

  // Serial.print("V & D'cm : ");
  // Serial.print(voltage);
  // Serial.print(" : ");
  // Serial.println(distance);

  return distance;
}

// float IRSensor::get_distance() {
//   // Convert ANalog value to distance in cm
//   // D = A * V^B
//   float voltage = static_cast<float>(__current_value) * (2.5 / 1023.0);
//   float distance = static_cast<float>(EMPIRICAL_CALIB) *
//                    pow(voltage, static_cast<float>(VOLTAGE_DROP));

//   Serial.print("IR Voltage: ");
//   Serial.print(voltage);
//   Serial.print(" V, Distance: ");
//   Serial.print(distance);
//   Serial.println(" cm");

//   return distance;
// }
