#include "main.h"

#include "leds.h"
#include "buttons.h"
#include "mesh.h"
#include "ble.h"

TaskHandle_t rfgTask;

void setup() {
  delay(400);
  // ESP_ERROR_CHECK(esp_task_wdt_init(10, false));
  setCpuFrequencyMhz(80);
  
  #ifdef RFG_DEBUG
    Serial.begin(115200);
    DEBUG_PRINTLN("Startup esp32");
  #endif

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

  initBLE();
  initMesh();
  initButtons();
  
  // Flash to indicate we're ready
  setupFlashLED();
}

void loop() {
  if (!isBleConnected()) {
    buttonTick();
  }
  ledTick();
  meshTick();
}

int vbatRead() {
  return 128;
}
