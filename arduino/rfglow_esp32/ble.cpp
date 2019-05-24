#include "ble.h"
#include "leds.h"

BLEServer *pServer = NULL;

boolean isConnected = false;

class BleRxCallbacks: public BLECharacteristicCallbacks {
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
          switchLedTo(target_hue, target_saturation, target_brightness);
          return;
        }

        unsigned int target_fade_ms = (data[4] << 8) | data[5];
        fadeLedTo(target_hue, target_saturation, target_brightness, target_fade_ms);
      } else {
        DEBUG_PRINTLN("Invalid BLE command length, ignoring.");
      }
    }
};

class BleServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    isConnected = true;
    stopAutocycle();
  };
 
  void onDisconnect(BLEServer* pServer) {
    isConnected = false;
  }
};

void initBLE() {
  // Create the BLE Device
  BLEDevice::init("RFGLOW");

  // Create the BLE Server
  pServer = BLEDevice::createServer();

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
    RX_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
	);

  pServer->setCallbacks(new BleServerCallbacks());
  pRxCharacteristic->setCallbacks(new BleRxCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

boolean isBleConnected() {
  return isConnected;
}