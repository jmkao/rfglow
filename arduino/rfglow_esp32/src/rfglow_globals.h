#include <Arduino.h>

#pragma once

/*** Tunables ***/
// Color change interval for button-controlled auto cycling
#define AUTO_INTERVAL 100

// Color update interval for BLE commands
#define LED_TICK_INTERVAL 10

// Interval between mesh commands to keep BLE off
#define WIFI_BLE_STOP_INTERVAL 5000

/*** Development Macros ***/
// Uncomment to enable debug logging
#define RFG_DEBUG

const String log_prefix = String("RFG: ");
#ifdef RFG_DEBUG
  #define DEBUG_PRINTLN(x) Serial.println (log_prefix + x)
  #define DEBUG_PRINT(x) Serial.print (log_prefix + x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

/*** Hardware Configuration ***/
#define BUTTON_PIN 23
#define VBAT_ADC_PIN 35

// Global typedefs
struct target_color {
  int h;
  int s;
  int v;
  unsigned long ms;
};


/*** Global Variable Definitions ***/

// Voltage indicator setup
extern volatile bool adcDone;

extern unsigned long lastCmdMs;