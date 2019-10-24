#include "main.h"

#include "leds.h"
#include "buttons.h"
#include "mesh.h"
#include "ble.h"

#include <esp_pm.h>

TaskHandle_t rfgTask;

void rfgLoopTask(void* pvParams) {
  while (1) {
    if (!isBleConnected()) {
      buttonTick();
    }
    ledTick();
    meshTick();

    unsigned long ms = millis();
    if (ms % 1000 == 0) {
      DEBUG_PRINTLN("vBAT = "+vbatRead());
    }
    delay(1);
  }
}

void setup() {
  analogReadResolution(11);
  analogSetAttenuation(ADC_11db);
  pinMode(VBAT_ADC_PIN, INPUT);

  

  delay(400);
  // ESP_ERROR_CHECK(esp_task_wdt_init(10, false));
  setCpuFrequencyMhz(80);

  // These don't seem to have any effect
  // esp_pm_config_esp32_t pm_config;
  // pm_config.max_freq_mhz = 80;
  // pm_config.min_freq_mhz = 20;
  // pm_config.light_sleep_enable = true;
  // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_pm_configure(&pm_config));
  
  #ifdef RFG_DEBUG
    Serial.begin(230400);
    DEBUG_PRINTLN("Startup esp32");
  #endif

  initLEDs();

  vbatLEDOn();

  initBLE();        // 130mA@2.9V for buttons+ble
  initMesh();       // 230mA@2.9V for buttons+mesh
  initButtons();    // 90mA@2.9V for buttons only
                    // 280mA@2.9V for buttons+ble+mesh
                    // 269mA@2.9V for buttons+ble+mesh+delay 1ms
                    // 35mA@2.9V for deep sleep
  
  // Flash to indicate we're ready
  setupFlashLED();

  //xTaskCreatePinnedToCore(rfgLoopTask, "rfgtask", 10000, NULL, 1, &rfgTask, 0);
}


void loop() {
  //vTaskSuspendAll();
  //delay(6000000L);
  rfgLoopTask(NULL);
}