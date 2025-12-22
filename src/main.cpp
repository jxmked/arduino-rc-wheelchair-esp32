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
#include "./controller.h"
#include "./ir_sensor.h"
#include "./structs.h"

SignalLED Signal_LED({
    .pin_gesture = LED_BLUE,
    .pin_override = LED_RED,
    .pin_lowbat = LED_YELLOW,
    .pin_bt = LED_GREEN,
});

Buzzer buzz(BUZZER_PIN);
Button btn_override(OVERRIDE_PUSH_BTN);
IRSensor sensor(IR_SENSOR_INPUT);

Controller mc;

Boot boot_anim;

TimeInterval deoverride_timer(5000, 0, true);
TimeInterval obst_clear(3000, 0, true);
TimeInterval obst_mtr_discon(750, 0, true);

bool is_obs_found = false;
bool is_override = false;
bool is_connected = false;

unsigned long last_data_ms = 0;
volatile static uint8_t remote_data;
volatile static uint8_t last_remote_data;

static BLERemoteCharacteristic* rem_chartc;
static BLEAdvertisedDevice* selected_device;

static BLEUUID SERVICE_UUID_B(SERVICE_UUID);
static BLEUUID CHARACTERISTIC_UUID_B(CHARACTERISTIC_UUID);

void S_LOG(String value) {
  static String last_string;

  if (last_string != value) {
    Serial.println(value);
    last_string = value;
  }
}

static void notifyCallback(BLERemoteCharacteristic* ret_rem_chartc,
                           uint8_t* data, size_t length, bool is_notify) {
  remote_data = data[0] & 0x0F;
  last_data_ms = millis();
}

class AdvertisedDevice_cb : public BLEAdvertisedDeviceCallbacks {
  // Begin scanning for remote
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID())
      if (advertisedDevice.isAdvertisingService(SERVICE_UUID_B)) {
        BLEDevice::getScan()->stop();
        selected_device = new BLEAdvertisedDevice(advertisedDevice);
      }
  }
};

class BLEState_cb : public BLEClientCallbacks {
  void onConnect(BLEClient* client) {
    is_connected = true;
    S_LOG("Connected to Server");
  }

  void onDisconnect(BLEClient* client) {
    is_connected = false;
    S_LOG("Disconnected from Server!");

    // The program hangs when it reaches this thing.
    // BLEDevice::getScan()->start(1);

    // This is better than rescan
    // Restart the system
    ESP.restart();
  }
};

bool connectToServer() {
  BLEClient* client = BLEDevice::createClient();
  client->connect(selected_device);
  client->setClientCallbacks(new BLEState_cb());

  BLERemoteService* rem_service = client->getService(SERVICE_UUID_B);
  if (rem_service == nullptr) return false;

  rem_chartc = rem_service->getCharacteristic(CHARACTERISTIC_UUID_B);
  if (rem_chartc == nullptr) return false;

  // Listen to notification instead of pooling...
  // Kinda better
  if (rem_chartc->canNotify()) {
    rem_chartc->registerForNotify(notifyCallback);
  }

  return true;
}

void setup() {
  Serial.begin(115200);

  analogReadResolution(ADC_BITS);

  // Built-in LED
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  BLEDevice::init("esp32-wheelchair-client");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDevice_cb());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  is_connected = connectToServer();

  if (!is_connected) {
    // If not getting connected
    // Wait for 3 Sec before restarting the system
    S_LOG("Handshake: FAILED");
    S_LOG("Restarting in 3 Seconds");

    delay(3000);
    ESP.restart();
  }

  boot_anim.begin();
  btn_override.begin();
  deoverride_timer.pause();
  mc.begin();
}

void loop() {
  // Check if we're still receiving data from bluetooth
  if (millis() - last_data_ms >= 250) {
    is_connected = false;
  }

  digitalWrite(2, is_connected ? HIGH : LOW);

  // Priority to obstacle detection
  sensor.update();
  mc.update();

  buzz.loop();

  if (boot_anim.is_animating()) {
    boot_anim.loop();
    mc.disconnect();
    return;
  }

  Signal_LED.update();

  if (!is_connected) {
    mc.stop();
    mc.disconnect();

    S_LOG("Bluetooth disconnected");

    Signal_LED.setState(E_SignalLED::BLUETOOTH, false);
    return;
  } else {
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);
  }

  if (btn_override.pressed()) {
    Signal_LED.offAll();
    mc.override();

    mc.update();

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
    is_obs_found = true;
    buzz.play(50, 100);

    obst_mtr_discon.reset();
    obst_mtr_discon.resume();
    obst_clear.reset();
    obst_clear.resume();

    mc.stop();

    Signal_LED.setState(E_SignalLED::GESTURE, true);
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    Signal_LED.setState(E_SignalLED::LOWBAT, true);
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);

    S_LOG("Obstacle Detected! Stopping Motors.");
  } else {
    Signal_LED.offAll();
  }

  if (is_obs_found) {
    // Stop the motor quickly then disconnect so we can
    // Move it freely
    if (obst_mtr_discon.marked()) {
      obst_mtr_discon.pause();
      mc.disconnect();
    }

    if (obst_clear.marked()) {
      obst_clear.pause();
      is_obs_found = false;
    }

    mc.update();

    return;
  }

  if (last_remote_data != remote_data) {
    Serial.print(remote_data, BIN);
    Serial.print(" : ");
    Serial.println(remote_data, HEX);
    last_remote_data = remote_data;
  }

  switch (remote_data) {
    case 0x1:  // Forward
      mc.forward();
      break;

    case 0x2:  // Reverse
      mc.reverse();
      break;

    case 0x8:  // Left
      mc.hard_left();
      break;

    case 0x4:  // Right
      mc.hard_right();
      break;

    case 0x9:  // Forward - Left
      mc.left();
      break;

    case 0x5:  // Forward - Right
      mc.right();
      break;

    case 0xA:  // Backward - Left
      mc.r_left();
      break;

    case 0x6:  // Backward - Right
      mc.r_right();
      break;

    default:
      mc.stop();
  }

  // Logging purposes
  switch (mc.state()) {
    case ControllerState::FORWARD:
      S_LOG("Motor: Forward");
      break;

    case ControllerState::REVERSE:
      S_LOG("Motor: Reverse");
      break;

    case ControllerState::HARD_LEFT:
      S_LOG("Motor: Rotate Left");
      break;

    case ControllerState::HARD_RIGHT:
      S_LOG("Motor: Rotate Right");
      break;

    case ControllerState::IDLE:
      S_LOG("Motor: Idle");
      break;

    case ControllerState::LEFT:
      S_LOG("Motor: Left");
      break;

    case ControllerState::R_LEFT:
      S_LOG("Motor: Reverse Left");
      break;

    case ControllerState::R_RIGHT:
      S_LOG("Motor: Right");
      break;

    case ControllerState::RIGHT:
      S_LOG("Motor: Right");
      break;

    case ControllerState::STOP:
      S_LOG("Motor: Stop");
      break;

    default:
      break;
  }

  if (mc.state() != ControllerState::IDLE &&
      mc.state() != ControllerState::STOP) {
    Signal_LED.setState(E_SignalLED::GESTURE, true);
  }
}
