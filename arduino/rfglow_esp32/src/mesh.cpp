#include "mesh.h"
#include "ble.h"
#include "leds.h"

#include <ArduinoJson.h>
#include <esp_wifi.h>

painlessMesh mesh;

long lastMeshCmdMs = INT32_MIN;
boolean isAPOnly = false;

void receivedCallback(uint32_t from, String &msg) {
  unsigned int tH, tS, tV, tMS;
  int len = msg.length();
  DEBUG_PRINTLN("Mesh received bytes: "+len);

  lastMeshCmdMs = millis();

/*
  if (len == 4 || len == 6) {
    //uint8_t data[6];
    //msg.getBytes(data, 6);
    uint8_t* data;
    data = (uint8_t*) msg.c_str();

    tH = (data[0] << 8) + data[1];
    tS = data[2];
    tV = data[3];

    if (len == 4) {
      DEBUG_PRINTLN("Mesh received data: "+data[0]+" "+data[1]+" "+data[2]+" "+data[3]);
      switchLedTo(tH, tS, tV);
      return;
    }

    tMS = (data[4] << 8) | data[5];
    fadeLedTo(tH, tS, tV, tMS);
  }
  */

/*
  StaticJsonDocument<128> doc;
  deserializeJson(doc, msg);
  tH = doc["h"];
  tS = doc["s"];
  tV = doc["v"];
  tMS = doc["ms"];
  */

  sscanf(msg.c_str(), "%d,%d,%d,%d", &tH, &tS, &tV, &tMS);

  if (tMS == 0) {
    switchLedTo(tH, tS, tV);
  } else {
    fadeLedTo(tH, tS, tV, tMS);
  }
}

void initMesh() {
  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | MSG_TYPES | REMOTE );
  mesh.init("RFGLOW", "rfglow3939", 5555, WIFI_MODE_APSTA);
  ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B));
  ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20));


  mesh.onReceive(&receivedCallback);
}

void reinitMeshAPOnly() {
  if (!isAPOnly) {
    DEBUG_PRINTLN("Re-initialize mesh to AP only");
    mesh.stop();
    mesh.init("RFGLOW", "rfglow3939", 5555, WIFI_MODE_AP);
    isAPOnly = true;
    ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20));
  }
}

void meshTick() {
  mesh.update();
  if (isMeshMasterPresent()) {
    if (!isBleConnected() && isBleStarted()) {
      stopBLE();
    }
  } else {
    if (!isBleStarted()) {
      startBLE();
    }
  }
}

boolean isMeshMasterPresent() {
  if (millis() - lastMeshCmdMs < WIFI_BLE_STOP_INTERVAL) {
    return true;
  }
  return false;
}

void setCommandToMesh(target_color target) {
  sendCommandToMesh(target.h, target.s, target.v, target.ms);
}

void sendCommandToMesh(unsigned int tH, unsigned int tS, unsigned int tV, unsigned int tMS) {
  String payload;

  /*
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

  DEBUG_PRINTLN("Mesh sent data: "+data[0]+" "+data[1]+" "+data[2]+" "+data[3]);

  String payload = String((char*)data);
  */

/*
  StaticJsonDocument<128> doc;
  doc["h"] = tH;
  doc["s"] = tS;
  doc["v"] = tV;
  doc["ms"] = tMS;
  serializeJson(doc, payload);
*/

  payload += tH;
  payload += ",";
  payload += tS;
  payload += ",";
  payload += tV;
  payload += ",";
  payload += tMS;

  mesh.sendBroadcast(payload);
}