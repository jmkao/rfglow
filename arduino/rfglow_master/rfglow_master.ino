#include <ClickButton.h>

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

// Repeat the send of commands this many times
#define COMMAND_REPEAT 2

ClickButton left(6, LOW, CLICKBTN_PULLUP);
ClickButton middle(5, LOW, CLICKBTN_PULLUP);
ClickButton right(4, LOW, CLICKBTN_PULLUP);

void setup()
{
  Serial.begin(38400);
  Serial.println("start");

  left.debounceTime   = 20;   // Debounce timer in ms
  left.multiclickTime = 50;  // Time limit for multi clicks
  left.longClickTime  = 1000; // time until "held-down clicks" register

  middle.debounceTime   = 20;   // Debounce timer in ms
  middle.multiclickTime = 50;  // Time limit for multi clicks
  middle.longClickTime  = 1000; // time until "held-down clicks" register

  right.debounceTime   = 20;   // Debounce timer in ms
  right.multiclickTime = 50;  // Time limit for multi clicks
  right.longClickTime  = 1000; // time until "held-down clicks" register


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

}


/*
char inData[80];
byte index = 0;
*/

// autocycle interval
#define INTERVAL 1000

unsigned long previousMs = 0;

int ledHue = 0;
unsigned char ledBrightnessLevels[] = { 64, 128, 192, 240 };
unsigned char NUM_BRIGHT_LEVELS = sizeof(ledBrightnessLevels);
unsigned char currentBrightnessIndex = 0;

boolean isAutoCycle = false;
boolean isOff = false;

void loop() {
  
  unsigned long currentMs = millis();
  
  left.Update();
  middle.Update();
  right.Update();
  
  if (left.clicks == 1) {
    Serial.println("Left button pressed.");
    if (ledHue < 60) {
      ledHue = ledHue - 60 + 360;
    } else {
      ledHue -= 60;
    }
    if (!isOff) {
      sendCurrentColor();
    }
  }
  
  if (middle.clicks == 1) {
    Serial.println("Middle button pressed.");
    if (!isOff) {
      isAutoCycle = !isAutoCycle;
    }
  }
  
  if (right.clicks == 1) {
    Serial.println("Right button pressed.");
    ledHue = (ledHue+60)%360;
    if (!isOff) {
      sendCurrentColor();
    }
  }
  
  // Negative clicks means long click
  // Middle button long press means toggle on/off
  if (middle.clicks < 0) {
    Serial.println("Middle button long press");
    isAutoCycle = false;
    isOff = !isOff;
    if (isOff) {
      sendCommand(0, 0, 0);
    } else {
      sendCurrentColor();
    }
  }
  
  // Left button long press means decrement brightness level
  if (left.clicks < 0) {
    Serial.println("Left button long press");
    if (currentBrightnessIndex < 1) {
      currentBrightnessIndex = 0;
    } else {
      currentBrightnessIndex--;
    }
    sendCurrentColor();
  }
  
  // Right button long press means increment brightness level
  if (right.clicks < 0) {
    Serial.println("Right button long press");
    if (currentBrightnessIndex >= (NUM_BRIGHT_LEVELS-1)) {
      currentBrightnessIndex = NUM_BRIGHT_LEVELS -1;
    } else {
      currentBrightnessIndex++;
    }
    sendCurrentColor();
  }
  
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

void sendHueChange(unsigned int hue) {
  sendCommand(hue, 255, ledBrightnessLevels[currentBrightnessIndex]);
}

void sendCurrentColor() {
  sendCommand(ledHue, 255, ledBrightnessLevels[currentBrightnessIndex]);
}

void sendCommand(unsigned int hue, unsigned int saturation, unsigned int brightness) {
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
 
  for (int i=0; i<COMMAND_REPEAT; i++) {
    if(cc1101.sendData(packet)){
      Serial.print("ok ");
    } else {
      Serial.print("failed ");
    }
  }
}

