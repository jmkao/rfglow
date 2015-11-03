#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#include "spip.h"
#include "EEPROM.h"
#include "cc1101.h"
#include "WString.h"
#include "calibration.h"


// My Debug Setup
//#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println (x)
  #define DEBUG_PRINT(x) Serial.print (x)
  #define DEBUG_PRINT2(x,y) Serial.print (x,y)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT2(x,y)
#endif

// CC1101 Setup
byte networkAddress[] = {39,0};
byte senderAddress = 4;
byte receiverAddress = 5;
//byte syncWord[] = {39,39};

#define RFCHANNEL 2

#define PA_EN_PIN 8
#define CC1101_CS_PIN 10

CC1101 cc1101;

// nRF8001 BLE setup
#define ADAFRUITBLE_RST 9
#define ADAFRUITBLE_REQ 4
#define ADAFRUITBLE_RDY 3
// Pin 3 is on interrupt 1 (Pin 2, the library default is on interrupt 0 but in use)
Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

int isBLEInit = 0;

// Update settings
#define MIN_LED_UPDATE_INTERVAL_MS 120
#define SENDS_ON_CHANGE 4
#define PERIODIC_RESEND_MS 1000

void setup()
{
  pinMode(PA_EN_PIN, OUTPUT);
  digitalWrite(PA_EN_PIN, LOW);

  //enterPanStampSPI();

  // Calibrate internal RC oscillator
  rcOscCalibrate();
  
  #ifdef DEBUG
    Serial.begin(38400);
  #endif
  DEBUG_PRINTLN("start");

  // initialize the RF Chip
  cc1101.setTxPowerAmp(0x80);
  cc1101.init();
  
  configRadio4800v();

  DEBUG_PRINTLN("radio initialized");

  DEBUG_PRINT("Sender address = ");
  DEBUG_PRINTLN(senderAddress);
  DEBUG_PRINT("Receiver address = ");
  DEBUG_PRINTLN(receiverAddress);
  
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

void rxCallback(uint8_t *buffer, uint8_t len)
{
  DEBUG_PRINT(F("Received "));
  DEBUG_PRINT(len);
  
  DEBUG_PRINT(F(" bytes: "));
  
  for(int i=0; i<len; i++)
   DEBUG_PRINT((char)buffer[i]); 

  DEBUG_PRINT(F(" ["));

  for(int i=0; i<len; i++) {
    DEBUG_PRINT(" 0x");
    DEBUG_PRINT2((char)buffer[i], HEX); 
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
    while (BTLEserial.available()) {
      BTLEserial.read();
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
 
  digitalWrite(PA_EN_PIN, HIGH);
  //delay(10);
  if(sendData(packet)){
    DEBUG_PRINT(">");
  } else {
    DEBUG_PRINT("X");
  }
  digitalWrite(PA_EN_PIN, LOW);
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
  digitalWrite(CC1101_CS_PIN, LOW);

  //SPCR = 0;
  //SPSR = 0;
  SPCR = _BV(SPE) | _BV(MSTR);
}

void leavePanStampSPI(void){
  // prepare SPI mode for ethernet
  digitalWrite(SPI_MOSI, LOW);
  digitalWrite(SPI_SCK, LOW);
  digitalWrite(CC1101_CS_PIN, HIGH);
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
      DEBUG_PRINTLN(F("Advertising started"));
      isBLEInit = 1;
      break;
    case ACI_EVT_CONNECTED:
      DEBUG_PRINTLN(F("Connected!"));
      break;
    case ACI_EVT_DISCONNECTED:
      DEBUG_PRINTLN(F("Disconnected or advertising timed out"));
      break;
    default:
      break;
  }
}

void configRadio4800v() {
  // settings for 4.8kbps variable packet size
  cc1101.writeReg(CC1101_IOCFG2,0x29);  //GDO2 Output Pin Configuration
  cc1101.writeReg(CC1101_IOCFG1,0x2E);  //GDO1 Output Pin Configuration
  cc1101.writeReg(CC1101_IOCFG0,0x06);  //GDO0 Output Pin Configuration
  cc1101.writeReg(CC1101_FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
  cc1101.writeReg(CC1101_SYNC1,0xD3);   //Sync Word, High Byte
  cc1101.writeReg(CC1101_SYNC0,0x91);   //Sync Word, Low Byte
  cc1101.writeReg(CC1101_PKTLEN,0xFF);  //Packet Length
  cc1101.writeReg(CC1101_PKTCTRL1,0x07);//Packet Automation Control
  cc1101.writeReg(CC1101_PKTCTRL0,0x45);//Packet Automation Control
  cc1101.writeReg(CC1101_CHANNR,0x00);  //Channel Number
  cc1101.writeReg(CC1101_FSCTRL1,0x06); //Frequency Synthesizer Control
  cc1101.writeReg(CC1101_FSCTRL0,0x00); //Frequency Synthesizer Control
  cc1101.writeReg(CC1101_FREQ2,0x23);   //Frequency Control Word, High Byte
  cc1101.writeReg(CC1101_FREQ1,0x31);   //Frequency Control Word, Middle Byte
  cc1101.writeReg(CC1101_FREQ0,0x3B);   //Frequency Control Word, Low Byte
  cc1101.writeReg(CC1101_MDMCFG4,0xC7); //Modem Configuration
  cc1101.writeReg(CC1101_MDMCFG3,0x83); //Modem Configuration
  cc1101.writeReg(CC1101_MDMCFG2,0x03); //Modem Configuration
  cc1101.writeReg(CC1101_MDMCFG1,0x22); //Modem Configuration
  cc1101.writeReg(CC1101_MDMCFG0,0xF8); //Modem Configuration
  cc1101.writeReg(CC1101_DEVIATN,0x40); //Modem Deviation Setting
  cc1101.writeReg(CC1101_MCSM2,0x07);   //Main Radio Control State Machine Configuration
  cc1101.writeReg(CC1101_MCSM1,0x30);   //Main Radio Control State Machine Configuration
  cc1101.writeReg(CC1101_MCSM0,0x18);   //Main Radio Control State Machine Configuration
  cc1101.writeReg(CC1101_FOCCFG,0x16);  //Frequency Offset Compensation Configuration
  cc1101.writeReg(CC1101_BSCFG,0x6C);   //Bit Synchronization Configuration
  cc1101.writeReg(CC1101_AGCCTRL2,0x43);//AGC Control
  cc1101.writeReg(CC1101_AGCCTRL1,0x40);//AGC Control
  cc1101.writeReg(CC1101_AGCCTRL0,0x91);//AGC Control
  cc1101.writeReg(CC1101_WOREVT1,0x87); //High Byte Event0 Timeout
  cc1101.writeReg(CC1101_WOREVT0,0x6B); //Low Byte Event0 Timeout
  cc1101.writeReg(CC1101_WORCTRL,0xF8); //Wake On Radio Control
  cc1101.writeReg(CC1101_FREND1,0x56);  //Front End RX Configuration
  cc1101.writeReg(CC1101_FREND0,0x10);  //Front End TX Configuration
  cc1101.writeReg(CC1101_FSCAL3,0xE9);  //Frequency Synthesizer Calibration
  cc1101.writeReg(CC1101_FSCAL2,0x2A);  //Frequency Synthesizer Calibration
  cc1101.writeReg(CC1101_FSCAL1,0x00);  //Frequency Synthesizer Calibration
  cc1101.writeReg(CC1101_FSCAL0,0x1F);  //Frequency Synthesizer Calibration
  cc1101.writeReg(CC1101_RCCTRL1,0x41); //RC Oscillator Configuration
  cc1101.writeReg(CC1101_RCCTRL0,0x00); //RC Oscillator Configuration
  cc1101.writeReg(CC1101_FSTEST,0x59);  //Frequency Synthesizer Calibration Control
  cc1101.writeReg(CC1101_PTEST,0x7F);   //Production Test
  cc1101.writeReg(CC1101_AGCTEST,0x3F); //AGC Test
  cc1101.writeReg(CC1101_TEST2,0x81);   //Various Test Settings
  cc1101.writeReg(CC1101_TEST1,0x35);   //Various Test Settings
  cc1101.writeReg(CC1101_TEST0,0x09);   //Various Test Settings
}

void softReset() {
  asm volatile ("  jmp 0");
}
