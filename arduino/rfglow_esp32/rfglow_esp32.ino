#include "rfglow_globals.h"
#include "buttons.h"
#include "leds.h"
#include "ble.h"
#include "mesh.h"

#include <driver/i2c.h>

void setup() {
  setCpuFrequencyMhz(80);
  
  #ifdef RFG_DEBUG
    Serial.begin(115200);
    DEBUG_PRINTLN("Startup esp32");
  #endif

  if (i2c_filter_enable(I2C_NUM_0, 1) == ESP_OK) {
    DEBUG_PRINTLN("Enabled ESP32 I2C filter");
  } else {
    DEBUG_PRINTLN("ESP32 I2C filter failed");
  }

  initLEDs();

  // Check battery voltage and show as color during init delays
  float vIn = (float)vbatRead() / 1000.0;
  if (vIn > 4.0) {
    setRGBRaw(0,0,15);
  } else if (vIn > 3.5) {
    setRGBRaw(0,15,0);
  } else if (vIn > 3.1) {
    setRGBRaw(7, 7, 0);
  } else {
    setRGBRaw(15, 0, 0);
  }
  delay(200);

  initButtons();
  initBLE();
  initMesh();
  
  // Flash to indicate we're ready
  setupFlashLED();
}

void loop() {
  // put your main code here, to run repeatedly
  if (!isBleConnected()) {
    buttonTick();
  }
  ledTick();
  meshTick();
}

static int vbatRead() {
  return 128;
}
