#include "EEPROM.h"
#include "cc1101.h"
#include "WString.h"

// CC1101 Setup
byte networkAddress[] = {39,0};
byte senderAddress = 4;
byte receiverAddress = 5;
byte syncWord[] = {39,39};

#define RFCHANNEL 2

CC1101 cc1101;

void setup()
{
  Serial.begin(38400);
  Serial.println("start");

  // initialize the RF Chip
  cc1101.init();
  cc1101.setChannel(RFCHANNEL, true);
  cc1101.setCarrierFreq(CFREQ_915);
  cc1101.setSyncWord(syncWord, true);

  cc1101.setDevAddress(senderAddress, true);
  cc1101.setTxPowerAmp(PA_LongDistance);
  Serial.println("device initialized");

  Serial.print("Sender address = ");
  Serial.println(senderAddress);
  Serial.print("Receiver address = ");
  Serial.println(receiverAddress);

  Serial.println("ready, enter 3 space spearated numbers like 30 30 30 for HSV:");
}


char inData[80];
byte index = 0;

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
}

void sendCommand(unsigned int hue, unsigned int saturation, unsigned int brightness) {
  CCPACKET packet;

  packet.length = 7;
  packet.data[0] = receiverAddress;

  packet.data[1] = (hue >> 8) & 0xFF;
  packet.data[2] = hue & 0xFF;
  packet.data[3] = (saturation >> 8) & 0xFF;
  packet.data[4] = saturation & 0xFF;
  packet.data[5] = (brightness >> 8) & 0xFF;
  packet.data[6] = brightness & 0xFF;
  
  Serial.println(packet.length);
  for (int i = 0; i < packet.length; i++) {
    Serial.println(packet.data[i]);
  }
 
  if(cc1101.sendData(packet)){
    Serial.print("ok ");
  } else {
    Serial.print("failed ");
  }
}

