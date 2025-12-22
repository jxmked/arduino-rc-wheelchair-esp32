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

IRSensor::IRSensor(uint16_t pin)
    : __pin(pin), __current_value(0.0), __sum_count(0), __timer(1, 0, true) {}

void IRSensor::update() {
  // This way, we can reduce the noise by averaging multiple samples
  // NOTE: KEEP THIS FUNCTION CALL FREQUENTLY IN THE MAIN LOOP
  if (__timer.marked()) {
    const uint16_t value = analogRead(__pin);

    __sum_value += value;
    __sum_count++;
  }

  if (__sum_count >= SAMPLING_SIZE) {
    __current_value =
        static_cast<float>(__sum_value) / static_cast<float>(SAMPLING_SIZE);

    __sum_value = 0;
    __sum_count = 0;
  }
}

bool IRSensor::isObstacleDetected() {
  const float distance = get_distance();
  const bool obs_detected =
      distance <= static_cast<float>(OBSTACLE_DISTANCE_THRESHOLD);

  return obs_detected;
}

float IRSensor::get_distance() {
  // Convert ANalog value to distance in cm
  // D = A * V^B
  const float voltage = static_cast<float>(__current_value) * (3.3 / 1023.0);
  const float distance = static_cast<float>(EMPIRICAL_CALIB) *
                         pow(voltage, static_cast<float>(VOLTAGE_DROP));

  // Serial.print("IR Voltage: ");
  // Serial.print(voltage);
  // Serial.print(" V, Distance: ");
  // Serial.print(distance);
  // Serial.println(" cm");

  return distance;
}
