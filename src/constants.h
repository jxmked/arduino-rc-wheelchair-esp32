#ifndef constant_h
#define constant_h

// May not be accurate to real measurement
#define OBSTACLE_DISTANCE_THRESHOLD 69.0  // cm

#define MOTOR_MAX_POWER 90  // 0 = No Power, 255 = Max Power (0-255)

// Slowly power on the motor so we don't stress the motor driver
#define MOTOR_ACCELERATION_TIME 3000  // ms

// Instead of start from 0, we start from this percetage
#define MOTOR_INTIAL_ACCELERATION 0.3  // Percentage

/** PINS **/
// Pin  --- conn
// All sense pin from esp32 is for input only. See the Esp32 Devtoolkit WROOM-32
// pinout
#define M1_L_IS 34  // Motor 1 Left Sense
#define M1_LPWM 25  // Motor 1 Left PWM

#define M1_R_IS 35  // Motor 1 Right Sense
#define M1_RPWM 26  // Motor 1 Right PWM

#define M2_L_IS 36  // Motor 2 Left Sense
#define M2_LPWM 32  // Motor 2 Left PWM

#define M2_R_IS 39  // Motor 2 Right Sense
#define M2_RPWM 33  // Motor 2 Right PWM

#define MOTOR_ENABLE_FLAG 4  // Toggle Motor Enable/Disable (For Both Motor)

#define IR_SENSOR_INPUT 13  // IR Sensor output data

#define SENSOR_DEACT_RATE 100  // counts

#define BUZZER_PIN 22  // BUZZER

#define LED_RED 16
#define LED_GREEN 17
#define LED_YELLOW 18
#define LED_BLUE 19

#define OVERRIDE_PUSH_BTN 21  // OVERRIDE PUSH BUTTON
/** END PINS **/

#define USE_MOTOR_DRIVER 1       // 1 = Motor driver, 0 = relay  (0, 1)
#define USE_MOTOR_PWM_CHANNEL 1  // Use PWM

// Sense restriction and default faulty protection
#define RSENSE 1000.0           // 1 kΩ resistor
#define IS_RATIO 8500.0         // BTS7960 datasheet ratio
#define ADC_REF 3.3             // ESP32 voltage reference
#define ADC_BITS 10             // resolution you choose (0-1023)
#define OVERCURRENT_LIMIT 28.0  // amps
#define FAULT_VOLTAGE 3.2       // V indicates fault
//

//
#define EMPIRICAL_CALIB 61.573
#define VOLTAGE_DROP -1.1068

// Number of samples for noise reduction.
// The higher, the smoother but less responsive
#define SAMPLING_SIZE 5


// Bluetooth UUID
// KEEP THIS SAME TO THE REMOTE SO THEY KNOW WHAT TO FIND
#define SERVICE_UUID "c973126d-5afe-4aec-ab49-f8ffbd14f059"
#define CHARACTERISTIC_UUID "9ca77c0d-14a4-403d-9677-0c2bf42b8194"

#endif