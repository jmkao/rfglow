#include "ble.h"
#include "leds.h"
#include "mesh.h"

BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLEAdvertising *pAdvertising  = NULL;

boolean isConnected = false;
boolean isStarted = false;

static class : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      uint16_t len = rxValue.length();
      DEBUG_PRINTLN("BLE received bytes: "+rxValue.length());

      uint8_t* data = (uint8_t*)rxValue.data();

      if (len == 4 || len == 6) {
        unsigned int target_hue = (data[0] << 8) + data[1];
        unsigned int target_saturation = data[2];
        unsigned int target_brightness = data[3];

        if (len == 4) {
          sendCommandToMesh(target_hue, target_saturation, target_brightness, 0);
          switchLedTo(target_hue, target_saturation, target_brightness);
          return;
        }

        unsigned int target_fade_ms = (data[4] << 8) | data[5];
        sendCommandToMesh(target_hue, target_saturation, target_brightness, target_fade_ms);
        fadeLedTo(target_hue, target_saturation, target_brightness, target_fade_ms);
      } else {
        DEBUG_PRINTLN("Invalid BLE command length, ignoring.");
      }
    }
} bleRxCallbacks;

static class : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    DEBUG_PRINTLN("BLE Client Connected");
    isConnected = true;
    stopAutocycle();
    reinitMeshAPOnly();
  };
 
  void onDisconnect(BLEServer* pServer) {
    DEBUG_PRINTLN("BLE Client Disconnected");
    isConnected = false;
  }
} bleServerCallbacks;

void initBLE() {
  DEBUG_PRINTLN("Init BLE");
  
  // Create the BLE Device
  BLEDevice::init("RFGLOW");
  ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N0));
  ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N0));
  BLEDevice::setPower(ESP_PWR_LVL_N0);


  // Create the BLE Server
  pServer = BLEDevice::createServer();

  // Create the BLE Service
  pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
    RX_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
	);

  pServer->setCallbacks(&bleServerCallbacks);
  pRxCharacteristic->setCallbacks(&bleRxCallbacks);
  pService->start();

  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  isStarted = true;
}

boolean isBleConnected() {
  return isConnected;
}

void stopBLE() {
  DEBUG_PRINTLN("Stopping BLE");
  // pAdvertising->stop();
  //pService->stop();
  if (BLEDevice::getInitialized()) {
    BLEDevice::deinit();
  }
  isStarted = false;
}

void startBLE() {
  DEBUG_PRINTLN("Starting BLE");
  //pService->start();
  //pAdvertising->start();
  if (!BLEDevice::getInitialized()) {
    initBLE();
  }
  isStarted = true;
}

boolean isBleStarted() {
  return isStarted;
}