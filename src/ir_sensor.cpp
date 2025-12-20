#include "./ir_sensor.h"

#include <Arduino.h>

#include "./TimeInterval.h"
#include "./constants.h"
#include "./structs.h"

IRSensor::IRSensor(int pin)
    : __pin(pin), __current_value(0.0), __sum_count(0), __timer(2, 0, true) {}

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
  float distance = get_distance();

  if (distance <= OBSTACLE_DISTANCE_THRESHOLD) {
    return true;
  }

  return false;
}

float IRSensor::get_distance() {
  // Convert ANalog value to distance in cm
  // D = A * V^B
  float voltage = __current_value * (3.3 / 4095.0);

  Serial.print("IR Sensor Voltage: ");
  Serial.print(voltage);
  float distance = static_cast<float>(EMPIRICAL_CALIB) *
                   pow(voltage, static_cast<float>(VOLTAGE_DROP));

  return distance;
}
