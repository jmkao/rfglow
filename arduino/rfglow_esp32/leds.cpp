#include "leds.h"

// TLC59116 Setup
TLC59116Manager tlcmanager(Wire, 30000L);
TLC59116* driver = NULL;

// LED state
unsigned int mH = 0;
unsigned int mS = 255;
unsigned int mB = 15;

unsigned int ma = 61;

// Targets for fading
unsigned int target_hue = 0, target_saturation = 0, target_brightness = 0, target_fade_ms = 0;
long fade_dH = 0, fade_dS = 0, fade_dV = 0;
unsigned long fade_start_ms = 0;

boolean isAutoCycle = false;

unsigned long prevMs = 0;

void initLEDs() {
  // Init TLC59116
  tlcmanager.init();
  DEBUG_PRINTLN("tlcmanager init complete");
  
  driver = &(tlcmanager[0]);
  driver->enable_outputs(true);
  DEBUG_PRINTLN("tlcmanager enable_outputs");
  driver->set_milliamps(ma);
}

void ledTick() {
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

void incrementHue() {
  isAutoCycle = false;
  mH = (mH + 60) % 360;
  setHSV(mH, mS, mB);
}

void toggleAutocycle() {
  isAutoCycle = !isAutoCycle;
}

void stopAutocycle() {
  isAutoCycle = false;
}

void incrementBrightness() {
  mB = (mB + 60) % 240;
  setHSV(mH, mS, mB);
}

void setupFlashLED() {
  driver->pattern(0b1100000000000000);
  delay(50);
  driver->pattern(0b0000011000000000);
  delay(50);
  driver->pattern(0b0000000000011000);
  delay(50);
  driver->pattern(0b0000000000000000);

  // driver->enable_outputs(false);  
}

void fadeTo(unsigned int h, unsigned int s, unsigned int v, unsigned int ms) {
  DEBUG_PRINTLN("Fade requested. Not yet implemented.");
}


void setHSV(unsigned int h, unsigned int s, unsigned int v) {
  DEBUG_PRINTLN("setHSV() called: "+h+" "+s+" "+v);
  unsigned char maLevel = h / 360;
  h = h % 360;

  //DEBUG_PRINTLN("maLevel = "+maLevel+", h = "+h);

  setHSVRaw(h,s,v,maLevel);
}

void setHSVRaw(unsigned int h, unsigned int s, unsigned int v, int maLevel) {
  
  //DEBUG_PRINTLN("setHSVRaw() called: "+h+" "+s+" "+v+" "+maLevel);
  
  unsigned char r, g, b;
  unsigned char region, remainder, p, q, t, newMa;

  switch (maLevel) {
    case 1:
      newMa = 61;
      break;
    case 2:
      newMa = 21;
      break;
    case 3:
      newMa = 10;
      break;
    case 0:
      newMa = 120;
      break;
    default:
      newMa = ma;
  }

  if (v == 0) {
    newMa = 0;
  }

  if (newMa != ma) {
    driver->set_milliamps(newMa);
    //DEBUG_PRINTLN("set_milliamps to "+newMa);
    ma = newMa;
  }


  if (ma == 120) {
    // Max possible brightness across multiple LEDs if at max power
    if (s == 0) {
      r = v;
      g = v;
      b = v;
    } else {
      region = h / 60;
      remainder = (h - (region * 60)) * 6; 
  
      p = (v * (255 - s)) >> 8;
      q = (v * (255 - ((s * remainder) >> 8))) >> 8;
      t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
  
      switch (region) {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        default:
            r = v; g = p; b = q;
            break;
      }
    }
  } else {
    // otherwise maintain constant brightness
    unsigned int tempH = h*96/45;
    uint8_t tempS = s;
    uint8_t tempV = v;
  
    uint8_t temp[5];
    uint8_t n = (tempH >> 8);
    //DEBUG_PRINTLN("Converted hue = "+tempH+" Color wheel section = "+n);
    temp[0] = temp[3] = (uint8_t)((                                        (tempS ^ 255)  * tempV) / 255);
    temp[1] = temp[4] = (uint8_t)((((( (tempH & 255)        * tempS) / 255) + (tempS ^ 255)) * tempV) / 255);
    temp[2] =          (uint8_t)(((((((tempH & 255) ^ 255) * tempS) / 255) + (tempS ^ 255)) * tempV) / 255);
    r  = temp[n + 2];
    g = temp[n + 1];
    b  = temp[n    ];
  }
  
  setRGBRaw(r, g, b);
}


void setRGBRaw(unsigned char r, unsigned char g, unsigned char b) {
  //DEBUG_PRINTLN("setRGBRaw() called: "+r+" "+g+" "+b);
  
  if (r==0 & g==0 & b==0) {
    driver->pattern(0x0000);
    driver->enable_outputs(false);
  } else {
    // if (!driver->is_enabled()) {
      driver->enable_outputs(true);
    // }
    
    byte leds[16];
    
    for (int i=0;i<5;i++) {
      // Set red channels
      leds[i] = r;
    }
    
    for (int i=5;i<10;i++) {
      // Set green channels
      leds[i] = g;
    }
    
    for (int i=11;i<16;i++) {
      // Set blue channels;
      leds[i] = b;
    }
    
    driver->set_outputs(leds);
  }
}
