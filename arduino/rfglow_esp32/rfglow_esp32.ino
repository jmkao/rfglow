#include "rfglow_globals.h"
#include "buttons.h"
#include "leds.h"

void setup() {
  setCpuFrequencyMhz(40);
  
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
  delay(500);

  initButtons();
  
  // Flash to indicate we're ready
  setupFlashLED();
}

void loop() {
  // put your main code here, to run repeatedly
  buttonTick();

  unsigned long curMs = millis();

  if (isAutoCycle) {
    if (curMs - prevMs > AUTO_INTERVAL) {
      mH = (mH + 60) % 360;
      setHSV(mH, mS, mB);
      prevMs = curMs;
    }
  } else {
    prevMs = curMs;
  } 
}

static int vbatRead() {
  return 128;
}
