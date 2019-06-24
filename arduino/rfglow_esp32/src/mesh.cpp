#include "mesh.h"
#include "ble.h"
#include "leds.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

union switch_cmd_t {
  struct color_t {
    uint16_t h;
    uint8_t s;
    uint8_t v;
  } color;

  uint8_t bytes[sizeof(color_t)];
};

union fade_cmd_t {
  struct color_t {
    uint16_t h;
    uint8_t s;
    uint8_t v;
    uint32_t fadeMs;
  } color;

  uint8_t bytes[sizeof(color_t)];
};


long lastMeshCmdMs = INT32_MIN;
uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void receivedCallback(const uint8_t *mac_addr, const uint8_t *data, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  DEBUG_PRINTLN("Mesh received "+len+" bytes from "+macStr);

  lastMeshCmdMs = millis();

  if (len == sizeof(switch_cmd_t)) {
    switch_cmd_t cmd;
    memcpy(&cmd.bytes, data, len);
    switchLedTo(cmd.color.h, cmd.color.s, cmd.color.v);
  } else if (len == sizeof(fade_cmd_t)) {
    fade_cmd_t cmd;
    memcpy(&cmd.bytes, data, len);
    fadeLedTo(cmd.color.h, cmd.color.s, cmd.color.v, cmd.color.fadeMs);
  } else {
    DEBUG_PRINTLN("Received byte count doesn't match any commands. Ignoring");
  }

  /*
  StaticJsonDocument<128> doc;
  deserializeJson(doc, msg);
  tH = doc["h"];
  tS = doc["s"];
  tV = doc["v"];
  tMS = doc["ms"];
  */

  // if (data[len-1] == 0) {
  //   sscanf((char*)data, "%d,%d,%d,%d", &tH, &tS, &tV, &tMS);
  // } else {
  //   DEBUG_PRINTLN("Illegal data packet received - doesn't end in \0. Ignoring.");
  //   return;
  // }

  // if (tMS == 0) {
  //   switchLedTo(tH, tS, tV);
  // } else {
  //   fadeLedTo(tH, tS, tV, tMS);
  // }

  DEBUG_PRINTLN("ESP32 Free Heap: "+ESP.getFreeHeap());
}

void sentCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  DEBUG_PRINTLN("Sent packet to: "+macStr);
  DEBUG_PRINTLN("Send Status: "+(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail"));
}

void initMesh() {
  DEBUG_PRINTLN("initMesh(): My MAC address is "+WiFi.macAddress());
  
  WiFi.mode(WIFI_STA);
  
  // bool status = WiFi.softAP("RFGLOW", NULL, WIFI_CHANNEL, 0);
  // DEBUG_PRINTLN("Wifi AP Init Status: "+status);

  WiFi.disconnect();

  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20));

  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_now_init());

  esp_now_register_recv_cb(&receivedCallback);
  esp_now_register_send_cb(&sentCallback);

  esp_now_peer_info_t peer;
  peer.channel = WIFI_CHANNEL;
  peer.ifidx = ESP_IF_WIFI_STA;
  peer.encrypt = false;
  memcpy(peer.peer_addr, broadcastMac, ESP_NOW_ETH_ALEN);
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_now_add_peer(&peer));
}

void meshTick() {
  if (isMeshMasterPresent()) {
    if (!isBleConnected() && isBleStarted()) {
      stopBLE();
    }
  } else {
    if (!isBleStarted()) {
      //startBLE();
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

  if (tMS == 0) {
    switch_cmd_t cmd;
    cmd.color.h = tH;
    cmd.color.s = tS;
    cmd.color.v = tV;

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_now_send(broadcastMac, cmd.bytes, sizeof(switch_cmd_t)));
  } else {
    fade_cmd_t cmd;
    cmd.color.h = tH;
    cmd.color.s = tS;
    cmd.color.v = tV;
    cmd.color.fadeMs = tMS;

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_now_send(broadcastMac, cmd.bytes, sizeof(fade_cmd_t)));
  }

  // uint8_t data[6];
  // size_t send_len = 6;
  
  // data[0] = (uint8_t) ((tH >> 8) & 0x00FF);
  // data[1] = (uint8_t) (tH & 0x00FF);
  // data[2] = (uint8_t) tS;
  // data[3] = (uint8_t) tV;

  // if (tMS == 0) {
  //   data[4] = 0;
  //   data[5] = 0;
  //   send_len = 4;
  // } else {
  //   data[4] = (uint8_t) ((tMS >> 8) & 0x00FF);
  //   data[5] = (uint8_t) (tMS & 0x00FF);
  // }

  // DEBUG_PRINTLN("Mesh sent data: "+data[0]+" "+data[1]+" "+data[2]+" "+data[3]);  


  // StaticJsonDocument<128> doc;
  // doc["h"] = tH;
  // doc["s"] = tS;
  // doc["v"] = tV;
  // doc["ms"] = tMS;
  // serializeJson(doc, payload);


  // String payload;
  // payload += tH;
  // payload += ",";
  // payload += tS;
  // payload += ",";
  // payload += tV;
  // payload += ",";
  // payload += tMS;
}