#include "ble.h"
#include "leds.h"

BLEServer *pServer = NULL;

class BleRxCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      DEBUG_PRINTLN("BLE received bytes: "+rxValue.length());
      uint8_t* data = (uint8_t*)rxValue.data();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
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

  pRxCharacteristic->setCallbacks(new BleRxCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}