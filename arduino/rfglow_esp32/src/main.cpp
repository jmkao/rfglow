#include "main.h"

#include "leds.h"
#include "buttons.h"
#include "mesh.h"
#include "ble.h"

#include <driver/i2c.h>
#include <esp_task_wdt.h>
#include <soc/timer_group_struct.h>
#include <soc/timer_group_reg.h>

TaskHandle_t rfgTask;

void setup() {
  delay(400);
  ESP_ERROR_CHECK(esp_task_wdt_init(10, false));
  //setCpuFrequencyMhz(160);
  
  #ifdef RFG_DEBUG
    Serial.begin(115200);
    DEBUG_PRINTLN("Startup esp32");
  #endif

  initLEDs();

  // // Check battery voltage and show as color during init delays
  // float vIn = (float)vbatRead() / 1000.0;
  // if (vIn > 4.0) {
  //   setRGBRaw(0,0,15);
  // } else if (vIn > 3.5) {
  //   setRGBRaw(0,15,0);
  // } else if (vIn > 3.1) {
  //   setRGBRaw(7, 7, 0);
  // } else {
  //   setRGBRaw(15, 0, 0);
  // }

  //initMesh();
  //initBLE();
  //initButtons();
  
  // Flash to indicate we're ready
  // setupFlashLED();

  delay(500);
  xTaskCreatePinnedToCore(core0Loop, "RFGTick", 10000, NULL, 1, &rfgTask, 0);
  esp_task_wdt_add(rfgTask);
  //vTaskSuspendAll();
}

void loop() {
  //vTaskSuspendAll();
  // put your main code here, to run repeatedly
  //if (!isBleConnected()) {
    //buttonTick();
  //}
  //ledTick();
  //meshTick();
  vTaskDelay(100);
  // TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  // TIMERG0.wdt_feed=1;
  // TIMERG0.wdt_wprotect=0;
}

int vbatRead() {
  return 128;
}

void core0Loop(void *params) {
  initButtons();
  initMesh();
  TRACE();

  while (1) {
    //DEBUG_PRINTLN("core0Loop tick");
    meshTick();
    buttonTick();
    ledTick();
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_task_wdt_reset());
    vTaskDelay(10);
  }
}
