#include "mesh.h"
#include "ble.h"
#include "leds.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

union fade_cmd_t {
  struct __attribute__((packed)) color_t {
    uint16_t h;
    uint8_t s;
    uint8_t v;
    uint16_t fadeMs;
    uint32_t seq;
  } color;

  uint8_t bytes[sizeof(color_t)];
};


long lastMeshCmdMs = INT32_MIN;
uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint32_t lastSeq = 0;
QueueHandle_t meshQueue;


void _espNowSendCmd(fade_cmd_t& cmd) {
  uint32_t seq = cmd.color.seq;

  cmd.color.h = htons(cmd.color.h);
  cmd.color.fadeMs = htons(cmd.color.fadeMs);
  cmd.color.seq = htonl(cmd.color.seq);

  DEBUG_PRINTLN("Sending "+sizeof(fade_cmd_t)+" bytes with seq: "+seq);
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_now_send(broadcastMac, cmd.bytes, sizeof(fade_cmd_t)));
}

void receivedCallback(const uint8_t *mac_addr, const uint8_t *data, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  DEBUG_PRINTLN("Mesh received "+len+" bytes from "+macStr);

  lastMeshCmdMs = millis();

  if (len == sizeof(fade_cmd_t)) {
    fade_cmd_t cmd;
    memcpy(cmd.bytes, data, sizeof(fade_cmd_t));
    cmd.color.h = ntohs(cmd.color.h);
    cmd.color.fadeMs = ntohs(cmd.color.fadeMs);
    cmd.color.seq = ntohl(cmd.color.seq);

    DEBUG_PRINTLN("lastSeq = "+lastSeq+", received seq = "+cmd.color.seq);

    if (cmd.color.seq == 0) {
      // Command has no sequence, process but don't resend or update lastSeq
    } else if (cmd.color.seq == 1 && lastSeq != 1) {
      // Controller in the network was reset
      DEBUG_PRINTLN("Received seq == 1, controller reset");
      lastSeq = cmd.color.seq;
    } else if (cmd.color.seq <= lastSeq) {
      // We've already received this command
      //DEBUG_PRINTLN("h = "+cmd.color.h+", s = "+cmd.color.s+", v="+cmd.color.v);
      DEBUG_PRINTLN("seq <= lastSeq - Skipping.");
      return;
    } else {
      lastSeq = cmd.color.seq;
    }

    xQueueSend(meshQueue, &cmd, 0);
  } else {
    DEBUG_PRINTLN("Received byte count doesn't match any commands. Ignoring");
  }

  DEBUG_PRINTLN("ESP32 Free Heap: "+ESP.getFreeHeap());
}

void sentCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
  //          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // DEBUG_PRINTLN("Sent packet to: "+macStr);
  //DEBUG_PRINTLN("Send Status: "+(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail"));
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

  meshQueue = xQueueCreate(5, sizeof(fade_cmd_t));
}

void meshTick() {
  fade_cmd_t cmd;
  if (xQueueReceive(meshQueue, &cmd, 0)) {
    if (cmd.color.fadeMs == 0) {
      switchLedTo(cmd.color.h, cmd.color.s, cmd.color.v);
    } else {
      fadeLedTo(cmd.color.h, cmd.color.s, cmd.color.v, cmd.color.fadeMs);
    }

    // Rebroadcast command if we processed it
    if (lastSeq == cmd.color.seq && cmd.color.seq != 0) {
      _espNowSendCmd(cmd);
    }

  }

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

void setCommandToMesh(target_color target, bool useSeq) {
  sendCommandToMesh(target.h, target.s, target.v, target.ms, useSeq);
}

void sendCommandToMesh(unsigned int tH, unsigned int tS, unsigned int tV, unsigned int tMS, bool useSeq) {

  uint32_t seq = 0;
  if (useSeq) {
    seq = ++lastSeq;
  }

  fade_cmd_t cmd;
  cmd.color.h = tH;
  cmd.color.s = tS;
  cmd.color.v = tV;
  cmd.color.fadeMs = tMS;
  cmd.color.seq = seq;

  _espNowSendCmd(cmd);
}