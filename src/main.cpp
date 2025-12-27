#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>

#include "./Button.h"
#include "./Buzzer.h"
#include "./Enums.h"
#include "./MotorController.h"
#include "./SignalLED.h"
#include "./TimeInterval.h"
#include "./boot.h"
#include "./constants.h"
#include "./ir_sensor.h"
#include "./structs.h"

static void logging_motor_data();
void S_LOG(String value);

SignalLED Signal_LED({
    .pin_gesture = LED_YELLOW,
    .pin_override = LED_RED,
    .pin_lowbat = LED_BLUE,
    .pin_bt = LED_GREEN,
});

uint8_t motor_action[16] = {};

Buzzer buzz(BUZZER_PIN);
Button btn_override(OVERRIDE_PUSH_BTN);
IRSensor sensor(IR_SENSOR_INPUT);

MotorController M1(M1_RPWM, M1_LPWM, MOTOR_ENABLE_FLAG, M1_R_IS, M1_L_IS);
MotorController M2(M2_RPWM, M2_LPWM, MOTOR_ENABLE_FLAG, M2_R_IS, M2_L_IS);

Boot boot_anim;

TimeInterval deoverride_timer(5000, 0, true);
TimeInterval obst_clear(3000, 0, true);
TimeInterval obst_mtr_discon(750, 0, true);

bool is_obs_found = false;
bool is_override = false;
bool is_connected = false;

// This variable will be use to smooth-out the motor
// initial acceleration. (Will be effective if the motor is set to PWM)
unsigned long mtr_smooth_accelrt_start = 0;
unsigned long mtr_smooth_accelrt_end = 0;

unsigned long last_data_ms = 0;
volatile static uint8_t remote_data;
volatile static uint8_t last_remote_data;

BLERemoteCharacteristic* rem_chartc;
BLEAdvertisedDevice* selected_device;
BLEClient* pClient;

static BLEUUID SERVICE_UUID_B(SERVICE_UUID);
static BLEUUID CHARACTERISTIC_UUID_B(CHARACTERISTIC_UUID);

static void notifyCallback(BLERemoteCharacteristic* _, uint8_t* data,
                           size_t length, bool is_notify) {
  // Since, our remote_data is an 8 bit, we can use the
  // rest of it for actual motor actions.
  // We need first to mask the response so we don't
  // mess with something we don't need
  remote_data = data[0] & 0x0F;
  remote_data |= motor_action[remote_data] << 4;

  last_data_ms = millis();
}

bool ble_reconnect() {
  BLEScan* pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);

  BLEScanResults devices = pScan->start(10);

  for (uint8_t i = 0; i < devices.getCount(); i++) {
    BLEAdvertisedDevice device = devices.getDevice(i);

    if (device.getName() == BLE_REMOTE_NAME) {
      selected_device = &device;
      // Break the loop and focus on the device we wanted to connected
      break;
    }
  }

  if (selected_device == nullptr) {
    ESP.restart();
    return false;
  }

  pClient = BLEDevice::createClient();
  pClient->connect(selected_device->getAddress());

  if (!pClient->isConnected()) {
    ESP.restart();
    return false;
  }

  BLERemoteService* p_remote_se =
      pClient->getService(selected_device->getServiceUUID());

  if (p_remote_se) {
    rem_chartc = p_remote_se->getCharacteristic(CHARACTERISTIC_UUID_B);
    if (rem_chartc && rem_chartc->canNotify()) {
      rem_chartc->registerForNotify(notifyCallback);
      return true;
    }
  }

  return false;
}

void setup() {
  Serial.begin(115200);

  analogReadResolution(ADC_BITS);

  // Built-in LED
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  BLEDevice::init("esp32-wheelchair-client");
  is_connected = ble_reconnect();

  if (!is_connected) {
    // If not getting connected
    // Wait for 3 Sec before restarting the system
    S_LOG("Handshake: FAILED");
    S_LOG("Restarting in 3 Seconds");

    delay(3000);
    ESP.restart();
  }

  // Motor actions - Structure
  // 2 bits for each motor
  // 10 = forward
  // 01 = reverse
  // 00 = stop
  // Motor 1 - Motor 2
  motor_action[0x1] = 0b1010;  // forward
  motor_action[0x2] = 0b0101;  // reverse;
  motor_action[0x4] = 0b1001;  // rotate_right;
  motor_action[0x8] = 0b0110;  // rotate_left;
  motor_action[0x5] = 0b1000;  // right;
  motor_action[0x9] = 0b0010;  // left;
  motor_action[0x6] = 0b0100;  // reverse right;
  motor_action[0xA] = 0b0001;  // reverse left;

  boot_anim.begin();
  btn_override.begin();
  deoverride_timer.pause();

  M1.stop();
  M1.disconnect(false);
  M2.stop();
  M2.disconnect(false);
}

void loop() {
  // Check if we're still receiving data from bluetooth
  if (millis() - last_data_ms >= 250) {
    is_connected = false;
  }

  digitalWrite(2, is_connected ? HIGH : LOW);

  // Priority to obstacle detection
  sensor.update();

  M1.update();
  M2.update();

  buzz.loop();

  Signal_LED.update();

  if (boot_anim.is_animating()) {
    boot_anim.loop();
    buzz.stop();

    return;
  }

  if (!is_connected) {
    /**
     * Conenction lost??
     *
     * Stop motors
     *
     * Turn Blluetooth indicator off
     *
     */
    M1.stop();
    M2.stop();
    M1.disconnect(false);
    M2.disconnect(false);

    buzz.stop();

    S_LOG("Bluetooth disconnected");

    Signal_LED.setState(E_SignalLED::BLUETOOTH, false);

    return;
  }

  Signal_LED.setState(E_SignalLED::BLUETOOTH, true);
  Signal_LED.update();

  if (btn_override.pressed()) {
    /**
     * Override button has been pressed
     *
     * Play buzzer sound
     *
     * Stop motors then release for a few seconds
     *
     * Turn override led on
     *
     */
    Signal_LED.offAll();
    M1.stop();
    M2.stop();

    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    S_LOG("Override Button Pressed! Motors Disconnected.");

    deoverride_timer.reset();
    deoverride_timer.resume();
    is_override = true;

    return;
  }

  if (is_override) {
    buzz.play(250, 250);

    if (deoverride_timer.marked()) {
      is_override = false;
      buzz.stop();
      deoverride_timer.pause();
      Signal_LED.setState(E_SignalLED::OVERRIDE, false);

      S_LOG("Override Period Ended. Motors Re-Enabled.");
    }

    return;
  }

  if (sensor.isObstacleDetected()) {
    /**
     * Obstacle found
     * Play Buzzer Sound
     *
     * Stop motors
     *
     * Turn Override led on
     *
     * This will continue to trigger when the obstacle remains
     * in the front. :)
     */
    is_obs_found = true;
    buzz.play(50, 100);

    obst_mtr_discon.reset();
    obst_mtr_discon.resume();
    obst_clear.reset();
    obst_clear.resume();

    M1.stop();
    M2.stop();

    Signal_LED.setState(E_SignalLED::GESTURE, true);
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    Signal_LED.setState(E_SignalLED::LOWBAT, true);
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);

    S_LOG("Obstacle Detected! Stopping Motors.");
  } else {
    Signal_LED.offAll();
  }

  if (is_obs_found) {
    // Obstacle found
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    M1.stop();
    M2.stop();

    // Stop the motor quickly then disconnect to unlock the motors
    if (obst_mtr_discon.marked()) {
      obst_mtr_discon.pause();
      M1.disconnect(false);
      M2.disconnect(false);
    }

    if (obst_clear.marked()) {
      obst_clear.pause();
      buzz.stop();
      Signal_LED.setState(E_SignalLED::OVERRIDE, false);

      is_obs_found = false;
      S_LOG("Cleared");
    }

    M1.update();
    M2.update();

    return;
  }

  /// MOTORS

  float mtr_smooth_accelrt_start = 0;
  bool mtr_just_started = false;

  // Check if only 1 motor is active so we can multiply the power of it
  // to able to move the wheelchair
  float mtr_power = 0;
  if ((remote_data & 0xF0) > 0) {
    if ((remote_data & 0x90) == 144 || (remote_data & 0x90) == 96) {
      mtr_power = 0.8;  // The power when running at opposite direction
    } else if ((remote_data & 0xA0) == 10 || (remote_data & 0x50) == 80) {
      mtr_power = 1.0;  // The power when running at the same direction
    } else {
      mtr_power = 1.4;  // The power when one 1 motor is running
    }

    // This will smooth the motor power.
    // Btw, this is linear.
    if (!mtr_just_started) {
      mtr_smooth_accelrt_start =
          millis() - static_cast<float>((MOTOR_ACCELERATION_TIME)) *
                         static_cast<float>((MOTOR_INTIAL_ACCELERATION));
      mtr_just_started = true;
    }

    const float diff =
        (static_cast<float>(millis()) - mtr_smooth_accelrt_start) /
        static_cast<float>((MOTOR_ACCELERATION_TIME));

    const float final_mtr_power = mtr_power * diff;

    // If the final motor power is higher than
    // motor power we need, prevent it.
    mtr_power = constrain(final_mtr_power, 0, mtr_power);
    /////////////////////////////////////

    M1.stop();
    M1.disconnect((remote_data & 0x30) > 0);
    if ((remote_data & 0xC0) > 0) {
      if ((remote_data & 0x40) > 0) {
        M1.reverse(mtr_power);
      } else if ((remote_data & 0x80) > 0) {
        M1.forward(mtr_power);
      }
    }

    M2.stop();
    M2.disconnect(((remote_data & 0xC0) > 0));
    if ((remote_data & 0x30) > 0) {
      if ((remote_data & 0x10) > 0) {
        M2.reverse(mtr_power);
      } else if ((remote_data & 0x20) > 0) {
        M2.forward(mtr_power);
      }
    }
  } else {
    mtr_just_started = false;
  }

  logging_motor_data();

  if ((M1.state() != MotorState::DISCONNECT ||
       M1.state() != MotorState::STOP) ||
      (M2.state() != MotorState::DISCONNECT ||
       M2.state() != MotorState::STOP)) {
    Signal_LED.setState(E_SignalLED::GESTURE, true);
  }
}

static void logging_motor_data() {
  // Logging purposes

  // This function tell us what we have receive and what controller does

  String str = F("BINRPNS|M1,M2: ");

  str = str + String(remote_data, BIN);
  str = str + F(" | ");

  // Motor 1
  if ((remote_data & 0xC0) > 0) {
    if ((remote_data & 0x40) > 0) {
      str = str + F("Reverse");
    } else if ((remote_data & 0x80) > 0) {
      str = str + F("Forward");
    }
  } else {
    str = str + F("STOP");
  }

  // Motor 2
  if ((remote_data & 0x30) > 0) {
    if ((remote_data & 0x10) > 0) {
      str = str + F(" | Reverse");
    } else if ((remote_data & 0x20) > 0) {
      str = str + F(" | Forward");
    }
  } else {
    str = str + F(" | STOP");
  }

  S_LOG(str);
}

void S_LOG(String value) {
  static String last_string;

  if (last_string != value) {
    Serial.println(value);
    last_string = value;
  }
}
