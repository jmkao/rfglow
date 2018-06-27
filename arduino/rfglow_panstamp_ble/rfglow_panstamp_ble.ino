#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include <avr/wdt.h>


// For PanStamp, these will only load if #include "HardwareSerial.h" is added to
// ~/.arduino15/packages/panstamp_avr/hardware/avr/1.5.7/cores/panstamp/Arduino.h
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
//#include <Adafruit_BluefruitLE_UART.h>

#include "BluefruitConfig.h"

#define DEBUG
String log_prefix = String("RFG: ");
#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println (log_prefix + x)
  #define DEBUG_PRINT(x) Serial.print (x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif


Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

#define MIN_LED_UPDATE_INTERVAL_MS 20
#define SENDS_ON_CHANGE 4
#define PERIODIC_RESEND_MS 1000

#define CC1101_RECV_ADDR CC1101_DEFVAL_ADDR

#define hopIntervalMicros 40000L
#define preHopMicros 10000L
bool isInPreHop = false;
unsigned long lastSendMicros = 0;
//byte fhss_schema[] = { 9, 1, 5, 6, 2, 3, 4, 0, 8, 7 };
//byte fhss_schema[] = { 1 };
//byte fhss_schema[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
byte fhss_schema[] = { 9, 17, 15, 11, 13, 8, 2, 10, 18, 12, 5, 4, 6, 19, 14, 1, 16, 7, 3, 0 };
byte ptr_fhss_schema = 0;

unsigned int packets_sent = 0;

CCPACKET gPacket;

unsigned long previousMs = 0;
unsigned int sends = 255;
unsigned int target_hue = 0, target_saturation = 0, target_brightness = 0, target_fade_ms = 0;
unsigned int current_hue = 0, current_saturation = 0, current_brightness = 0;
long fade_dH = 0, fade_dS = 0, fade_dV = 0;
unsigned long fade_start_ms = 0;

void setup() {
  //pinMode(15, OUTPUT);
  //digitalWrite(15, HIGH);

  panstamp.radio.enableHGM();
  panstamp.radio.setTxState();
  panstamp.setHighTxPower();
  panstamp.cc1101.setChannel(fhss_schema[ptr_fhss_schema]);

  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Startup serial");
  #endif
  
  if ( !ble.begin(VERBOSE_MODE) ) {
    DEBUG_PRINTLN(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  DEBUG_PRINTLN( F("OK!") );
//  if (!ble.factoryReset()) {
//    DEBUG_PRINTLN("Couldn't factory reset");
//  }

  //ble.info();
  ble.sendCommandCheckOK("AT+GAPDEVNAME=RFGLOW");
  ble.sendCommandCheckOK("ATZ");
  ble.sendCommandCheckOK("AT+GAPCONNECTABLE=1");
  ble.sendCommandCheckOK("AT+GAPDEVNAME");

  //ble.setBleUartRxCallback(rxCallback);
  //ble.setConnectCallback(connectCallback);
  //ble.setDisconnectCallback(disconnectCallback);
  
  ble.setMode(BLUEFRUIT_MODE_DATA);
  ble.setTimeout(5);

  lastSendMicros = micros();

  wdt_enable(WDTO_120MS);
}

void loop() {

  // FHSS interrupt processing
  if(!isInPreHop && micros() - lastSendMicros > hopIntervalMicros - preHopMicros) {
    ptr_fhss_schema++;  // Increment pointer of fhss schema array to perform next channel change
    if(ptr_fhss_schema >= sizeof(fhss_schema)) {
      ptr_fhss_schema=0; // To avoid array indexing overflow
      //DEBUG_PRINTLN("Packets sent: "+packets_sent);
      packets_sent = 0;
    }
    panstamp.cc1101.setChannel(fhss_schema[ptr_fhss_schema]); // Change channel
    isInPreHop = true;
  }

  unsigned long currentMicros = micros();

  // Send only if Int_TX_cnt number of interrupts have been fired
  if (currentMicros - lastSendMicros > hopIntervalMicros) {
    sendCurrentColor();
    lastSendMicros = currentMicros;
    //DEBUG_PRINTLN("Sent at: "+currentMicros);
    isInPreHop = false;
  }
  
  if (ble.available()) {
    uint8_t buffer[32];
    int len;
    len = ble.readBLEUart(buffer, 31);
    rxCallback(buffer, len);
  }

  wdt_reset();
}

void rxCallback(uint8_t *buffer, uint16_t len) {

  DEBUG_PRINTLN("Received "+len+" bytes...");

//  for(int i=0; i<len; i++)
//   DEBUG_PRINT((char)buffer[i]); 

//  DEBUG_PRINT(F(" ["));

//  for(int i=0; i<len; i++) {
//    DEBUG_PRINT(" 0x");
//    DEBUG_PRINT((char)buffer[i] + HEX); 
//  }
//  DEBUG_PRINT(F(" ]"));
//  
//  DEBUG_PRINTLN("");

  if (len == 4 || len == 6) {
    // 4 bytes means treat as 1 16-bit and 2 8-bit uints directly
    target_hue = (buffer[0] << 8) + buffer[1];
    target_saturation = buffer[2];
    target_brightness = buffer[3];
    sends = 0;
    DEBUG_PRINTLN("Directly read: HSV=("+target_hue+"h, "+target_saturation+"s, "+target_brightness+"v)");
    if (len == 6) {
      target_fade_ms = (buffer[4] << 8) | buffer[5];
      fade_start_ms = millis();
      DEBUG_PRINTLN("Read fade_ms: "+target_fade_ms);

      if (current_hue == 360) {
        if (target_hue < 180) {
          current_hue = 0;
        }
      } else if (current_hue == 0) {
        if (target_hue >= 180) {
          current_hue = 360;
        }
      }

      if (target_hue == 360) {
        if (current_hue < 180) {
          target_hue = 0;
        }
      } else if (target_hue == 0) {
        if (current_hue >= 180) {
          target_hue = 360;
        }
      }
    
      fade_dH = (int)target_hue - (int)current_hue;
      fade_dS = (int)target_saturation - (int)current_saturation;
      fade_dV = (int)target_brightness - (int)current_brightness;
      DEBUG_PRINTLN("Fade deltas dH, dS, dV = "+fade_dH+", "+fade_dS+", "+fade_dV);

    } else {
      target_fade_ms = 0;
      fade_start_ms = 0;
    }
  } else if (len <= 11) {
    // safe number of bytes in the packet
    char inData[len+1];

    memcpy(inData, buffer, len);
    inData[len] = 0;
    
    unsigned int hue, saturation, brightness;
    int numExtracted = sscanf(inData,"%u %u %u",&hue,&saturation,&brightness);
    DEBUG_PRINT("Parsed fields: ");
    DEBUG_PRINTLN(numExtracted);
    if (numExtracted == 3) {
      target_hue = hue;
      target_saturation = saturation;
      target_brightness = brightness;
      sends = 0;
      DEBUG_PRINT("Parsed values: ");
      DEBUG_PRINT(target_hue);
      DEBUG_PRINT(", ");
      DEBUG_PRINT(target_saturation);
      DEBUG_PRINT(", ");
      DEBUG_PRINTLN(target_brightness);
    }
  } else {
    // Buffer has some illegal amount of data
    while (ble.available()) {
      ble.read();
    }
    DEBUG_PRINTLN("Trashed unparseable packet");
  }
}

void sendCurrentColor() {

  if (fade_start_ms == 0 || target_fade_ms == 0) {
    current_hue = target_hue;
    current_saturation = target_saturation;
    current_brightness = target_brightness;
    fade_start_ms = 0;
    target_fade_ms = 0;
  } else if (target_fade_ms == 65535) {
    // Code for UO decay
  } else if (millis() - fade_start_ms > 60000 || millis() - fade_start_ms > target_fade_ms) {
    // Max fade is 60 seconds, terminate the transition.
    // Or we exceeded the fade target ms, also terminate the transition
    
    current_hue = target_hue;
    current_saturation = target_saturation;
    current_brightness = target_brightness;
    fade_start_ms = 0;
    target_fade_ms = 0;
  } else {
    // In the middle of a linear fade
    long dT = millis() - fade_start_ms;
    long ratio = 10000 - (dT * 10000 / target_fade_ms);
    DEBUG_PRINTLN("Fade at "+ratio/100+"%");
    int dH = ratio * fade_dH / 10000;
    int dS = ratio * fade_dS / 10000;
    int dV = ratio * fade_dV / 10000;

    current_hue = target_hue - dH;
    current_saturation = target_saturation - dS;
    current_brightness = target_brightness - dV;
  }
    
  sendCommand(current_hue, current_saturation, current_brightness);
}

void sendCommand(uint16_t hue, uint8_t saturation, uint8_t brightness) {

  DEBUG_PRINTLN("Send HSV: "+hue+", "+saturation+", "+brightness);

  unsigned long startTime = micros();

  gPacket.length = 5;
  gPacket.data[0] = CC1101_RECV_ADDR;
  gPacket.data[1] = (hue >> 8) & 0xFF;
  gPacket.data[2] = hue & 0xFF;
  gPacket.data[3] = saturation;
  gPacket.data[4] = brightness;

  /*
  packet.data[1] = (hue >> 8) & 0xFF;
  packet.data[2] = hue & 0xFF;
  packet.data[3] = (saturation >> 8) & 0xFF;
  packet.data[4] = saturation & 0xFF;
  packet.data[5] = (brightness >> 8) & 0xFF;
  packet.data[6] = brightness & 0xFF;
  */

  /*
  Serial.println(packet.length);
  for (int i = 0; i < packet.length; i++) {
    Serial.println(packet.data[i]);
  }
  */
 
  //digitalWrite(PA_EN_PIN, HIGH);
  //delay(10);
  if(sendData(gPacket)){
    DEBUG_PRINT(">");
    packets_sent++;
  } else {
    DEBUG_PRINT("X");
  }

  //DEBUG_PRINTLN("Send took " + (micros()-startTime) + " us");
  //DEBUG_PRINTLN("Sent packet on channel "+panstamp.cc1101.channel);

}

boolean sendData(CCPACKET packet) {
  boolean result = panstamp.cc1101.sendData(packet);
  return result;
}
