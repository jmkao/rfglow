#include "EEPROM.h"
#include "cc1101.h"
#include "WString.h"

#include <SPI.h>
#include <Adafruit_BLE_UART.h>

// CC1101 Setup
byte networkAddress[] = {39,0};
byte senderAddress = 4;
byte receiverAddress = 5;
byte syncWord[] = {39,39};

#define RFCHANNEL 2

CC1101 cc1101;

// nRF8001 BLE setup
#define ADAFRUITBLE_RST 9
#define ADAFRUITBLE_REQ 4
#define ADAFRUITBLE_RDY 3
// Pin 3 is on interrupt 1 (Pin 2, the library default is on interrupt 0 but in use)
Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

// Update settings
#define MAX_LED_UPDATE_INTERVAL_MS 20
#define SENDS_ON_CHANGE 4
#define PERIODIC_RESEND_MS 10000

void setup()
{
  Serial.begin(38400);
  Serial.println("start");

  // initialize the RF Chip
  cc1101.init();
  cc1101.setCarrierFreq(CFREQ_868);

 cc1101.writeReg(CC1101_PKTLEN,0x5);  //Packet Length
//  cc1101.writeReg(CC1101_PKTCTRL1,0x04);//Packet Automation Control
  cc1101.writeReg(CC1101_PKTCTRL0,0x45);//Packet Automation Control
//  cc1101.writeReg(CC1101_ADDR,0x00);    //Device Address
//  cc1101.writeReg(CC1101_CHANNR,0x00);  //Channel Number
//  cc1101.writeReg(CC1101_FSCTRL1,0x06); //Frequency Synthesizer Control
//  cc1101.writeReg(CC1101_FSCTRL0,0x00); //Frequency Synthesizer Control
//  cc1101.writeReg(CC1101_FREQ2,0x21);   //Frequency Control Word, High Byte
  cc1101.writeReg(CC1101_FREQ1,0x80);   //Frequency Control Word, Middle Byte
  cc1101.writeReg(CC1101_FREQ0,0x00);   //Frequency Control Word, Low Byte
  cc1101.writeReg(CC1101_MDMCFG4,0xC7); //Modem Configuration
  cc1101.writeReg(CC1101_MDMCFG3,0x02); //Modem Configuration
  cc1101.writeReg(CC1101_MDMCFG2,0x13); //Modem Configuration
  cc1101.writeReg(CC1101_MDMCFG1,0x80); //Modem Configuration
//  cc1101.writeReg(CC1101_MDMCFG0,0xF8); //Modem Configuration
//  cc1101.writeReg(CC1101_DEVIATN,0x34); //Modem Deviation Setting

  cc1101.setTxPowerAmp(PA_LongDistance);

  cc1101.setChannel(RFCHANNEL, true);
  cc1101.setSyncWord(syncWord, true);

  cc1101.setDevAddress(senderAddress, true);

  Serial.println("device initialized");

  Serial.print("Sender address = ");
  Serial.println(senderAddress);
  Serial.print("Receiver address = ");
  Serial.println(receiverAddress);
  
  leavePanStampSPI();

  BTLEserial.setRXcallback(rxCallback);
  BTLEserial.setACIcallback(aciCallback);
  BTLEserial.setDeviceName("RFGLOW");
  BTLEserial.begin();
}

unsigned long previousMs = 0;
unsigned int sends = 255;
unsigned int c_hue = 0, c_saturation = 0, c_brightness = 0;

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

void loop() {
  unsigned long currentMs = millis();
  
  // Tell the nRF8001 to do whatever it should be working on.
  BTLEserial.pollACI();
  
  // Only check for a pending update if more than LED_UPDATE_INTERVAL_MS
  // has passed since we last sent an update
  if (currentMs - previousMs > MAX_LED_UPDATE_INTERVAL_MS) {
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

void rxCallback(uint8_t *buffer, uint8_t len)
{
  Serial.print(F("Received "));
  Serial.print(len);
  
  Serial.print(F(" bytes: "));
  
  for(int i=0; i<len; i++)
   Serial.print((char)buffer[i]); 

  Serial.print(F(" ["));

  for(int i=0; i<len; i++) {
    Serial.print(" 0x"); Serial.print((char)buffer[i], HEX); 
  }
  Serial.print(F(" ]"));
  
  Serial.println();

  if (len == 4) {
    // 4 bytes means treat as 1 16-bit and 2 8-bit uints directly
    c_hue = (buffer[0] << 8) + buffer[1];
    c_saturation = buffer[2];
    c_brightness = buffer[3];
    sends = 0;
    Serial.print("Directly read: ");
    Serial.print(c_hue);
    Serial.print(", ");
    Serial.print(c_saturation);
    Serial.print(", ");
    Serial.println(c_brightness);
  } else if (len <= 11) {
    // safe number of bytes in the packet
    char inData[len+1];

    memcpy(inData, buffer, len);
    inData[len] = 0;
    
    unsigned int hue, saturation, brightness;
    int numExtracted = sscanf(inData,"%u %u %u",&hue,&saturation,&brightness);
    Serial.print("Parsed fields: ");
    Serial.println(numExtracted);
    if (numExtracted == 3) {
      c_hue = hue;
      c_saturation = saturation;
      c_brightness = brightness;
      sends = 0;
      Serial.print("Parsed values: ");
      Serial.print(c_hue);
      Serial.print(", ");
      Serial.print(c_saturation);
      Serial.print(", ");
      Serial.println(c_brightness);
    }
  } else {
    // Buffer has some illegal amount of data
    while (BTLEserial.available()) {
      BTLEserial.read();
    }
    Serial.println("Trashed unparseable packet");
  }
}


void sendCurrentColor() {
  sendCommand(c_hue, c_saturation, c_brightness);
}

void sendCommand(uint16_t hue, uint8_t saturation, uint8_t brightness) {
  CCPACKET packet;

  packet.length = 5;
  packet.data[0] = receiverAddress;
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
 
  if(sendData(packet)){
    //Serial.print(">");
  } else {
    //Serial.print("X");
  }
}

boolean sendData(CCPACKET packet) {
  noInterrupts();
  enterPanStampSPI();
  boolean result = cc1101.sendData(packet);
  leavePanStampSPI();
  interrupts();
  return result;
}

void enterPanStampSPI(void){
  // prepare SPI mode for panStamp
  digitalWrite(SPI_SCK, LOW);
  digitalWrite(SPI_MOSI, LOW);
  digitalWrite(ADAFRUITBLE_REQ, HIGH);
  digitalWrite(SPI_SS, LOW);
  SPCR = _BV(SPE) | _BV(MSTR);
}

void leavePanStampSPI(void){
  // prepare SPI mode for ethernet
  digitalWrite(SPI_MOSI, LOW);
  digitalWrite(SPI_SCK, LOW);
  digitalWrite(SPI_SS, HIGH);
  digitalWrite(ADAFRUITBLE_REQ, LOW);
  
  SPCR = 0;
  SPSR = 0;
  SPSR = (1<<SPI2X);
  SPCR = ((1<<SPE)|(1<<MSTR)|(0<<SPR1) | (1<<SPR0) | (1<<DORD) | (0<<CPOL) | (0<<CPHA));
}

void aciCallback(aci_evt_opcode_t event)
{
  switch(event)
  {
    case ACI_EVT_DEVICE_STARTED:
      Serial.println(F("Advertising started"));
      break;
    case ACI_EVT_CONNECTED:
      Serial.println(F("Connected!"));
      break;
    case ACI_EVT_DISCONNECTED:
      Serial.println(F("Disconnected or advertising timed out"));
      break;
    default:
      break;
  }
}

