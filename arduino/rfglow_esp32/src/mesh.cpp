#include "mesh.h"

painlessMesh mesh;

unsigned long lastMeshCmdMs = 0;

void receivedCallback(uint32_t from, String &msg) {
  unsigned int tH, tS, tV, tMS;
  int len = msg.length();

  lastMeshCmdMs = millis();

  if (len == 4 || len == 6) {
    uint8_t data[6];
    msg.getBytes(data, 6);

    tH = (data[0] << 8) + data[1];
    tS = data[2];
    tV = data[3];

    if (len == 4) {
      switchLedTo(tH, tS, tV);
      return;
    }

    tMS = (data[4] << 8) | data[5];
    fadeLedTo(tH, tS, tV, tMS);
  }
}

void initMesh() {
  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  mesh.init("RFGLOW", "rfglow3939");
  mesh.onReceive(&receivedCallback);
}

void meshTick() {
  mesh.update();
  if (millis() - lastMeshCmdMs < WIFI_BLE_STOP_INTERVAL) {
    if (!isBleConnected()) {
      stopBLE();
    }
  }
}

void sendCommandToMesh(unsigned int tH, unsigned int tS, unsigned int tV, unsigned int tMS) {
  unsigned char data[7];
  
  data[0] = (uint8_t) ((tH >> 8) & 0x00FF);
  data[1] = (uint8_t) (tH & 0x00FF);
  data[2] = (uint8_t) tS;
  data[3] = (uint8_t) tV;

  if (tMS == 0) {
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
  } else {
    data[4] = (uint8_t) ((tMS >> 8) & 0x00FF);
    data[5] = (uint8_t) (tMS & 0x00FF);
    data[6] = 0;
  }

  String payload = String((char*)data);
  mesh.sendBroadcast(payload);
}