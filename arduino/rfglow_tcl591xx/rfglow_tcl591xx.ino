#include <avr/pgmspace.h>
#include "HardwareSerial.h"

#include <OneButton.h>

//#include <cc1101.h>
//#include <panstamp.h>

#define TLC59116_LOWLEVEL 0
#define TLC59116_DEV 0
#include <TLC59116.h>

// Logging Setup
#define RFG_DEBUG

String log_prefix = String("RFG: ");

#ifdef RFG_DEBUG
  #define DEBUG_PRINTLN(x) Serial.println (log_prefix + x)
  #define DEBUG_PRINT(x) Serial.print (log_prefix + x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

// TLC59116 Setup
TLC59116Manager tlcmanager;
TLC59116* driver = NULL;

// CC1101 Setup
boolean packetAvailable = false;

// Power saving #defines
#define power_adc_enable()      (PRR &= (uint8_t)~(1 << PRADC))
#define power_adc_disable()     (PRR |= (uint8_t)(1 << PRADC))

#define power_spi_enable()      (PRR &= (uint8_t)~(1 << PRSPI))
#define power_spi_disable()     (PRR |= (uint8_t)(1 << PRSPI))

#define power_usart0_enable()   (PRR &= (uint8_t)~(1 << PRUSART0))
#define power_usart0_disable()  (PRR |= (uint8_t)(1 << PRUSART0))

#define power_timer0_enable()   (PRR &= (uint8_t)~(1 << PRTIM0))
#define power_timer0_disable()  (PRR |= (uint8_t)(1 << PRTIM0))

#define power_timer1_enable()   (PRR &= (uint8_t)~(1 << PRTIM1))
#define power_timer1_disable()  (PRR |= (uint8_t)(1 << PRTIM1))

#define power_timer2_enable()   (PRR &= (uint8_t)~(1 << PRTIM2))
#define power_timer2_disable()  (PRR |= (uint8_t)(1 << PRTIM2))

#define power_twi_enable()      (PRR &= (uint8_t)~(1 << PRTWI))
#define power_twi_disable()     (PRR |= (uint8_t)(1 << PRTWI))

// OneButton Setup
OneButton button(19, true);
//ClickButton button(4, LOW, CLICKBTN_PULLUP);

void cc1101Interrupt(void) {
  packetAvailable = true;
  TCNT1 = 0; // Reset Interrupt counter 
}

unsigned long last_rx_time = 0;
bool fhss_on = false;  // Syncronization flag with PTX (at setup time must be false)

#define hopIntervalMicros 40000L
#define preHopMicros 10000L
unsigned long lastHopMicros = 0;
boolean isInPreHop = false;
//byte fhss_schema[] = { 9, 1, 5, 6, 2, 3, 4, 0, 8, 7 };
//byte fhss_schema[] = { 1 };
//byte fhss_schema[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
byte fhss_schema[] = { 9, 17, 15, 11, 13, 8, 2, 10, 18, 12, 5, 4, 6, 19, 14, 1, 16, 7, 3, 0 };
byte ptr_fhss_schema = 0;
unsigned int packets_received = 0;

#define RF_LOCKOUT_MS 4000
unsigned long lastCmdMs = 0;

unsigned int mH = 0;
unsigned int mS = 255;
unsigned int mB = 15;

unsigned int ma = 61;

int clickState = 0;

#define AUTO_INTERVAL 100
boolean isAutoCycle = false;
unsigned long prevMs = 0;

void setup() {
  
  #ifdef RFG_DEBUG
    Serial.begin(115200);
    Serial.println("Startup serial");
  #endif
  DEBUG_PRINTLN("TCL591XX Test");
  //DEBUG_PRINTLN("setup()");

  power_adc_disable();
  //power_timer0_disable();
  //power_timer1_disable();
  //power_timer2_disable();
  
  // Init CC1101
  panstamp.cc1101.setTxPowerAmp(0x03);
  panstamp.cc1101.setChannel(fhss_schema[ptr_fhss_schema]);

  last_rx_time = micros();
  
  panstamp.cc1101.setRxState();
  attachInterrupt(0, cc1101Interrupt, FALLING);

  // Init OneButton
  button.attachClick(clickAction);
  button.attachDoubleClick(doubleClickAction);
  button.attachLongPressStart(longPressAction);
  button.setClickTicks(150);
  /*
  button.debounceTime = 20;
  button.multiclickTime = 85;
  button.longClickTime = 1000;
  */

  // Init TLC59116
  tlcmanager.init();
  
  //sleepDriver(false);
  driver = &(tlcmanager[0]);
  driver->enable_outputs(true);
  driver->set_milliamps(ma);
  
  
  driver->pattern(0b1100000000000000);
  delay(50);
  driver->pattern(0b0000011000000000);
  delay(50);
  driver->pattern(0b0000000000011000);
  delay(50);
  driver->pattern(0b0000000000000000);

  driver->enable_outputs(false);  

}

void loop() {
  
  if (!isInPreHop && micros() - lastHopMicros > hopIntervalMicros - preHopMicros) {    
    if(fhss_on==true) {  // Only if we are synced with PTX and if it's time to perform channel change (10ms before expected data from PTX)
      ptr_fhss_schema++;  // Increment pointer of fhss schema array to perform next channel change
      if(ptr_fhss_schema >= sizeof(fhss_schema)) {
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

//  The following code serves to declare "out of sync" of the receiver if we don't receive data for a time needed to cover the entire channels sequency schema (plus a little extra delay)
//  and change channel in case this one is pertrubated for a long time
//  In this mode I'm able to resyncronize PTX and PRX in any case (Reset of PTX, reset of PRX, channels perturbation, ecc.) and very quickly
  if((currentMicros - last_rx_time) > ((((sizeof(fhss_schema))+5)*hopIntervalMicros))) { 
    //DEBUG_PRINTLN("FHSS resync. Haven't seen data in micros: "+(currentMicros - last_rx_time));
    DEBUG_PRINTLN("FHSS resync.");
    last_rx_time = micros();
    fhss_on=false;
    ptr_fhss_schema++;
    if(ptr_fhss_schema >= sizeof(fhss_schema)) {
      ptr_fhss_schema=0;
    }
    panstamp.cc1101.setChannel(fhss_schema[ptr_fhss_schema]);
  }

  
  if(packetAvailable) {
    packetAvailable = false;

    fhss_on=true; // Now we can follow the fhss schema (we are synced with PTX channel and with interrupt time of PTX)
    last_rx_time = micros();  // Update received time
    lastHopMicros = last_rx_time - preHopMicros;
    //DEBUG_PRINTLN("Received packet at: "+last_rx_time);

    CCPACKET packet;
    
    detachInterrupt(0);
    
    if(panstamp.cc1101.receiveData(&packet) > 0) {
      if (packet.crc_ok && packet.length > 1) {
        //DEBUG_PRINTLN("Received packet on channel "+panstamp.cc1101.channel);

        #ifdef RFG_DEBUG
          int16_t rssi;
          if (packet.rssi >= 128) {
            rssi = (int16_t)((int16_t)( packet.rssi - 256) / 2) - 74;
          } else {
            rssi = (packet.rssi / 2) - 74; 
          }
          DEBUG_PRINTLN("Receipt packet with RSSI: "+rssi);
        #endif

        packets_received++;
        processData(packet.data);
      }
    }
    attachInterrupt(0, cc1101Interrupt, FALLING);
  }

  //if (curMs - lastCmdMs > RF_LOCKOUT_MS) {
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

