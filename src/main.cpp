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
bool is_override = false;
bool is_connected = false;

volatile static uint8_t remote_data;
unsigned long last_data_ms = 0;

static BLERemoteCharacteristic* rem_chartc;
static BLEAdvertisedDevice* selected_device;

static BLEUUID SERVICE_UUID_B(SERVICE_UUID);
static BLEUUID CHARACTERISTIC_UUID_B(CHARACTERISTIC_UUID);

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
    Serial.println("Connected to Server");
  }

  void onDisconnect(BLEClient* client) {
    is_connected = false;
    Serial.println("Disconnected from Server!");

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
    Serial.println("Handshake: FAILED");
    Serial.println("Restarting in 3 Seconds");

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

  // const auto res = sensor.isObstacleDetected();

  // Serial.println(res);
  // return;

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
    Signal_LED.setState(E_SignalLED::BLUETOOTH, false);
    return;
  } else {
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);
  }

  if (btn_override.pressed()) {
    Signal_LED.offAll();
    mc.override();

    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    Serial.println("Override Button Pressed! Motors Disconnected.");

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
      Serial.println("Override Period Ended. Motors Re-Enabled.");
    }

    return;
  }

  if (sensor.isObstacleDetected()) {
    mc.stop();

    Signal_LED.setState(E_SignalLED::GESTURE, true);
    Signal_LED.setState(E_SignalLED::OVERRIDE, true);
    Signal_LED.setState(E_SignalLED::LOWBAT, true);
    Signal_LED.setState(E_SignalLED::BLUETOOTH, true);

    // Serial.println("Obstacle Detected! Stopping Motors.");

    // return;
  } else {
    Signal_LED.offAll();
  }

  // //******* For Testing *******//

  // const uint16_t left_right_value = analogRead(LEFT_RIGHT_PIN);
  // const uint16_t front_back_value = analogRead(FRONT_BACK_PIN);
  // const int __y = (int)left_right_value - 512;
  // const int __x = (int)front_back_value - 512;

  // int x = 0;
  // int y = 0;

  // if (__x <= -510) {
  //   x = -1;
  // } else if (__x >= 510) {
  //   x = 1;
  // }

  // if (__y <= -510) {
  //   y = -1;
  // } else if (__y >= 510) {
  //   y = 1;
  // }

  // if (y == 1) {
  //   if (x == 1) {
  //     mc.right();
  //   } else if (x == -1) {
  //     mc.left();
  //   } else {
  //     mc.hard_right();
  //   }
  // } else if (y == -1) {
  //   if (x == -1) {
  //     mc.right();
  //   } else if (x == 1) {
  //     mc.left();
  //   } else {
  //     mc.hard_left();
  //   }
  // } else {
  //   if (x == 1) {
  //     mc.forward();
  //   } else if (x == -1) {
  //     mc.reverse();
  //   }
  // }

  // if (y == 0 && x == 0) {
  //   mc.stop();
  // }

  if (mc.state() != ControllerState::IDLE &&
      mc.state() != ControllerState::STOP) {
    Signal_LED.setState(E_SignalLED::GESTURE, true);
  }
}
