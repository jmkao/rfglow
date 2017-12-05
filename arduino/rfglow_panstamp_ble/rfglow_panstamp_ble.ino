#include <string.h>
#include <Arduino.h>
#include <SPI.h>

// For PanStamp, these will only load if #include "HardwareSerial.h" is added to
// ~/.arduino15/packages/panstamp_avr/hardware/avr/1.5.7/cores/panstamp/Arduino.h
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
//#include <Adafruit_BluefruitLE_UART.h>

#include "BluefruitConfig.h"

//#define DEBUG
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

unsigned long previousMs = 0;
unsigned int sends = 255;
unsigned int c_hue = 0, c_saturation = 0, c_brightness = 0;

void setup() {
  //pinMode(15, OUTPUT);
  //digitalWrite(15, HIGH);

  panstamp.radio.enableHGM();
  panstamp.setHighTxPower();

  #ifdef DEBUG
    Serial.begin(38400);
    Serial.println("Startup serial");
  #endif
  DEBUG_PRINTLN("Panstamp NRF51 Test");
  DEBUG_PRINTLN("setup()");
  
  if ( !ble.begin(VERBOSE_MODE) ) {
    DEBUG_PRINTLN(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  DEBUG_PRINTLN( F("OK!") );

//  if (!ble.factoryReset()) {
//    DEBUG_PRINTLN("Couldn't factory reset");
//  }

  ble.info();
  ble.sendCommandCheckOK("AT+GAPDEVNAME=RFGLOW");
  ble.sendCommandCheckOK("ATZ");
  ble.sendCommandCheckOK("AT+GAPCONNECTABLE=1");
  ble.sendCommandCheckOK("AT+GAPDEVNAME");

  //ble.setBleUartRxCallback(rxCallback);
  //ble.setConnectCallback(connectCallback);
  //ble.setDisconnectCallback(disconnectCallback);
  
  ble.setMode(BLUEFRUIT_MODE_DATA);
}

void loop() {
  // put your main code here, to run repeatedly
  if (ble.available()) {
    uint8_t buffer[4];
    int len;
    len = ble.readBLEUart(buffer, 4);
    rxCallback(buffer, len);
  }

  unsigned long currentMs = millis();
  
  // Only check for a pending update if more than LED_UPDATE_INTERVAL_MS
  // has passed since we last sent an update
  if (currentMs - previousMs > MIN_LED_UPDATE_INTERVAL_MS) {
    if (sends < SENDS_ON_CHANGE) {
      // We have a change where we have not yet sent all our requested updates
      sendCurrentColor();
      sends++;
      previousMs = currentMs;
    } else if (currentMs - previousMs > PERIODIC_RESEND_MS) {
      // It's been more than the periodic resend interval
      sendCurrentColor();
      previousMs = currentMs;
    } else {
      // We have no pending changes and it's not yet time for a periodic
      // resend, so do nothing
    }
  }

}

void rxCallback(uint8_t *buffer, uint16_t len) {
  DEBUG_PRINT(F("Received "));
  DEBUG_PRINT(len);
  
  DEBUG_PRINT(F(" bytes: "));
  
  for(int i=0; i<len; i++)
   DEBUG_PRINT((char)buffer[i]); 

  DEBUG_PRINT(F(" ["));

  for(int i=0; i<len; i++) {
    DEBUG_PRINT(" 0x");
    DEBUG_PRINT((char)buffer[i] + HEX); 
  }
  DEBUG_PRINT(F(" ]"));
  
  DEBUG_PRINTLN("");

  if (len == 4) {
    // 4 bytes means treat as 1 16-bit and 2 8-bit uints directly
    c_hue = (buffer[0] << 8) + buffer[1];
    c_saturation = buffer[2];
    c_brightness = buffer[3];
    sends = 0;
    DEBUG_PRINT("Directly read: ");
    DEBUG_PRINT(c_hue);
    DEBUG_PRINT(", ");
    DEBUG_PRINT(c_saturation);
    DEBUG_PRINT(", ");
    DEBUG_PRINTLN(c_brightness);
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
      c_hue = hue;
      c_saturation = saturation;
      c_brightness = brightness;
      sends = 0;
      DEBUG_PRINT("Parsed values: ");
      DEBUG_PRINT(c_hue);
      DEBUG_PRINT(", ");
      DEBUG_PRINT(c_saturation);
      DEBUG_PRINT(", ");
      DEBUG_PRINTLN(c_brightness);
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
  sendCommand(c_hue, c_saturation, c_brightness);
}

void sendCommand(uint16_t hue, uint8_t saturation, uint8_t brightness) {
  CCPACKET packet;

  packet.length = 5;
  packet.data[0] = CC1101_RECV_ADDR;
  packet.data[1] = (hue >> 8) & 0xFF;
  packet.data[2] = hue & 0xFF;
  packet.data[3] = saturation;
  packet.data[4] = brightness;

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
  if(sendData(packet)){
    DEBUG_PRINT(">");
  } else {
    DEBUG_PRINT("X");
  }
  //digitalWrite(PA_EN_PIN, LOW);
}

boolean sendData(CCPACKET packet) {
  boolean result = panstamp.cc1101.sendData(packet);
  return result;
}

