#include "rfglow_tcl591xx.h"
#include "rfglow_globals.h"

void setup() {
  
  #ifdef RFG_DEBUG
    Serial.begin(115200);
    Serial.println("Startup serial");
  #endif
  DEBUG_PRINTLN("TCL591XX Test");

  // Init TLC59116
  tlcmanager.init();
  driver = &(tlcmanager[0]);
  driver->enable_outputs(true);
  driver->set_milliamps(ma);

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

  // Init CC1101
  panstamp.cc1101.setTxPowerAmp(0x03);
  panstamp.cc1101.setChannel(fhss_schema[ptr_fhss_schema]);
  panstamp.cc1101.setRxState();
  attachInterrupt(0, cc1101Interrupt, FALLING);

  // Init OneButton
  button.attachClick(clickAction);
  button.attachDoubleClick(doubleClickAction);
  button.attachLongPressStart(longPressAction);
  button.setClickTicks(150);

  // Init global state
  power_adc_disable();
  last_rx_time = micros();

  // Flash to indicate we're ready
  setupFlashLED();
}

void loop() {
  
  if (!isInPreHop && micros() - lastHopMicros > hopIntervalMicros - preHopMicros) {    
    if (fhss_on==true) {
      ptr_fhss_schema++;  // Increment pointer of fhss schema array to perform next channel change
      if (ptr_fhss_schema >= sizeof(fhss_schema)) {
        ptr_fhss_schema=0; // To avoid fhss schema array indexing overflow
        //DEBUG_PRINTLN("Packets received: "+packets_received);
        packets_received = 0;
      }
      panstamp.cc1101.setChannel(fhss_schema[ptr_fhss_schema]); // Change channel
      isInPreHop = true;
      
      //DEBUG_PRINTLN("FHSS changed channel to: "+panstamp.cc1101.channel);
      //DEBUG_PRINTLN("Hopped channel at: "+micros());
    }
  }

  unsigned long currentMicros = micros();
  unsigned long curMs = millis();
  
  if (currentMicros - lastHopMicros > hopIntervalMicros) {
    lastHopMicros = currentMicros;
    isInPreHop = false;
  }
  
  // Detect when ptr_fhss_schema is out of sync with transmitter, then recover by
  // switching channel to ptr_fhss_schema = 0 and wait until we hear something
  if ((currentMicros - last_rx_time) > ((((sizeof(fhss_schema))+5)*hopIntervalMicros))) { 
    //DEBUG_PRINTLN("FHSS resync. Haven't seen data in micros: "+(currentMicros - last_rx_time));
    DEBUG_PRINTLN("FHSS resync.");
    last_rx_time = micros();
    fhss_on=false;
    ptr_fhss_schema++;
    if (ptr_fhss_schema >= sizeof(fhss_schema)) {
      ptr_fhss_schema=0;
    }
    panstamp.cc1101.setChannel(fhss_schema[ptr_fhss_schema]);
  }

  
  if (packetAvailable) {
    packetAvailable = false;

    fhss_on=true; // Now we can follow the fhss schema (we are synced with PTX channel and with interrupt time of PTX)
    last_rx_time = micros();  // Update received time
    lastHopMicros = last_rx_time - preHopMicros;
    //DEBUG_PRINTLN("Received packet at: "+last_rx_time);

    CCPACKET packet;
    
    detachInterrupt(0);
    
    if (panstamp.cc1101.receiveData(&packet) > 0) {
      if (packet.crc_ok && packet.length > 1) {
        //DEBUG_PRINTLN("Received packet on channel "+panstamp.cc1101.channel);

        #ifdef RFG_DEBUG
          int16_t rssi;
          if (packet.rssi >= 128) {
            rssi = (int16_t)((int16_t)( packet.rssi - 256) / 2) - 74;
          } else {
            rssi = (packet.rssi / 2) - 74; 
          }
          DEBUG_PRINTLN("Received packet with RSSI: "+rssi);
        #endif

        packets_received++;
        processData(packet.data);
      }
    }
    attachInterrupt(0, cc1101Interrupt, FALLING);
  }

  if (fhss_on == false) {
    button.tick();
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
}

void cc1101Interrupt(void) {
  packetAvailable = true;
  TCNT1 = 0; // Reset Interrupt counter 
}

void clickAction() {
  DEBUG_PRINTLN("Single click");
  isAutoCycle = false;
  mH = (mH + 60) % 360;
  setHSV(mH, mS, mB);
}

void doubleClickAction() {
  DEBUG_PRINTLN("Double click");
  isAutoCycle = !isAutoCycle;
}

void longPressAction() {
  DEBUG_PRINTLN("Long press");
  mB = (mB + 60) % 240;
  setHSV(mH, mS, mB);
}

void processData(byte *data) {

  unsigned int hue;
  unsigned int saturation;
  unsigned int brightness;

  hue = ((data[1] << 8) + data[2]);
  saturation = data[3];
  brightness = data[4];

  setHSV(hue, saturation, brightness);
  
  lastCmdMs = millis();
}

void setHSV(unsigned int h, unsigned int s, unsigned int v) {
  //DEBUG_PRINTLN("setHSV() called: "+h+" "+s+" "+v);
  setHSVRaw(h,s,v);
}

void setHSVRaw(unsigned int h, unsigned int s, unsigned int v) {
  
  //DEBUG_PRINTLN("setHSVRaw() called: "+h+" "+s+" "+v);
  
  unsigned char r, g, b;
  unsigned char region, remainder, p, q, t, maLevel, newMa;

  maLevel = h / 360;
  h = h % 360;

  //DEBUG_PRINTLN("maLevel = "+maLevel+", h = "+h);

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
    default:
      newMa = 120;
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
    if (!driver->is_enabled()) {
      driver->enable_outputs(true);
    }
    
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

ISR(ADC_vect) { adcDone = true; }

static int vccRead (byte count =4) {
  set_sleep_mode(SLEEP_MODE_ADC);
  ADMUX = bit(REFS0) | 14; // use VCC and internal bandgap
  bitSet(ADCSRA, ADIE);
  while (count-- > 0) {
    adcDone = false;
    while (!adcDone)
      sleep_mode();
  }
  bitClear(ADCSRA, ADIE);  
  
  // convert ADC readings to fit in one byte, i.e. 20 mV steps:
  //  1.0V = 0, 1.8V = 40, 3.3V = 115, 5.0V = 200, 6.0V = 250
  // return (55U * 1023U) / (ADC + 1) - 50;
  int vcc =  (1100L * 1023) / ADC;
  DEBUG_PRINTLN("Regulated Vcc: "+vcc);

  return vcc;
}

static int vbatRead() {
  byte count = 4;
  
  set_sleep_mode(SLEEP_MODE_ADC);
  ADMUX = 1 << REFS0 | 0; // use VCC and A0
  bitSet(ADCSRA, ADIE);
  while (count-- > 0) {
    adcDone = false;
    while (!adcDone)
      sleep_mode();
  }
  bitClear(ADCSRA, ADIE);  

  int vbatRaw = ADC;
  DEBUG_PRINTLN("Vbat raw ADC value: "+vbatRaw);

  int vcc = vccRead();
  
  int vbat = (long)vcc * vbatRaw * 2 / 1023;
  DEBUG_PRINTLN("Vbat: "+vbat);

  return vbat;
}
