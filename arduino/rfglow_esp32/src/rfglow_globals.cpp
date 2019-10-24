#include "rfglow_globals.h"

/*** Global Variable Initialization ***/
bool adcDone = false;

unsigned long lastCmdMs = 0;

/*** Global Functions ***/
float vbatRead() {
  int raw = 0;
  
  for (int i=0; i<20 ; i++) {
    raw += analogRead(VBAT_ADC_PIN);
  }

  DEBUG_PRINTLN("Raw VBAT_ADC_PIN ADC read: "+raw);

  float vBat = 0.0;

  vBat = (float)raw / 20.0 / 4096.0 * 2.0 * 7.451;

  return vBat;
}
