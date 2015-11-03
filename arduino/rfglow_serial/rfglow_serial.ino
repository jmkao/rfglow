#include "EEPROM.h"
#include "cc1101.h"
#include "WString.h"
#include "calibration.h"

// CC1101 Setup
byte networkAddress[] = {39,0};
byte senderAddress = 4;
byte receiverAddress = 5;
//byte syncWord[] = {39,39};

#define RFCHANNEL 2

#define PA_EN_PIN 8

CC1101 cc1101;

void setup()
{
  pinMode(PA_EN_PIN, OUTPUT);
  digitalWrite(PA_EN_PIN, LOW);

  rcOscCalibrate();
  
  Serial.begin(38400);
  Serial.println("start");

  // Init CC1101
  cc1101.setTxPowerAmp(0x80);
  cc1101.init();
  //cc1101.reset();
  //cc1101.setCarrierFreq(CFREQ_915);

  configRadio4800v();

  Serial.println("device initialized");

  Serial.print("Sender address = ");
  Serial.println(senderAddress);
  Serial.print("Receiver address = ");
  Serial.println(receiverAddress);

  Serial.println("ready, enter 3 space spearated numbers like 30 30 30 for HSV:");
}


char inData[80];
byte index = 0;

// autocycle interval
#define INTERVAL 1000

unsigned long previousMs = 0;

int ledHue = 0;
unsigned char ledBrightnessLevels[] = { 64, 128, 192, 240 };
unsigned char NUM_BRIGHT_LEVELS = sizeof(ledBrightnessLevels);
unsigned char currentBrightnessIndex = 0;

boolean isAutoCycle = true;

void loop() {
  while (Serial.available() > 0) {
    char aChar = Serial.read();
    if (aChar == '\n') {
      // Turn inData into a \0 terminated string
      inData[index] = 0;

      Serial.print("Confirmed ");
      Serial.print(inData);
      Serial.print(". "); 
      
      // Parse the line
      int hue, saturation, brightness;
      int numExtracted = sscanf(inData,"%u %u %u",&hue,&saturation,&brightness);
      Serial.print(" # values = ");
      Serial.println(numExtracted);
      if (numExtracted == 3) {
        sendCommand(hue, saturation, brightness);
      }
      
      // Reset index
      index = 0;
      Serial.println("ready, enter 3 space separated numbers like 30 30 30 for HSV:");
    } else {
      inData[index] = aChar;
      index++;
    }
  }
  unsigned long currentMs = millis();

  if (isAutoCycle) {
    if (currentMs - previousMs > INTERVAL) {
      ledHue = (ledHue+60)%360;
      sendCurrentColor();
      previousMs = currentMs;
    }
  } else {
    previousMs = currentMs;
  }
}

void sendCurrentColor() {
  sendCommand(ledHue, 255, ledBrightnessLevels[currentBrightnessIndex]);
}


void sendCommand(uint16_t hue, uint8_t saturation, uint8_t brightness) {
  CCPACKET packet;

  packet.length = 5;
  packet.data[0] = receiverAddress;
  packet.data[1] = (hue >> 8) & 0xFF;
  packet.data[2] = hue & 0xFF;
  packet.data[3] = saturation;
  packet.data[4] = brightness;
  
  Serial.println(packet.length);
  for (int i = 0; i < packet.length; i++) {
    Serial.println(packet.data[i]);
  }
 
  digitalWrite(PA_EN_PIN, HIGH);
  if(cc1101.sendData(packet)){
    Serial.print("ok ");
  } else {
    Serial.print("failed ");
  }
  digitalWrite(PA_EN_PIN, LOW);
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
