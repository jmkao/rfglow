#include <Wire.h>
#include <avr/pgmspace.h> 

#define TLC59116_LOWLEVEL 1
//#define TLC59116_DEV 1

#include "TLC59116.h"


TLC59116Manager tlcmanager;
TLC59116* driver = NULL;

boolean driverSleep = true;


String log_prefix = String("RFG: ");

#define LOG(x) Serial.println(log_prefix + x)

void setup() {
  Serial.begin(38400);
  LOG("TCL591XX Test");
  LOG("setup()");
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  tlcmanager.init();

  sleepDriver(true);
}

void loop() {
  static char inData[80];
  static byte index = 0;

  while (Serial.available() > 0) {
    char aChar = Serial.read();
    if (aChar == '\n') {
      // Turn inData into a \0 terminated string
      inData[index] = 0;

      Serial.print("Confirmed ");
      Serial.print(inData);
      Serial.print(". "); 
      
      // Parse the line
      int channel, value;
      int numExtracted = sscanf(inData,"%u %u",&channel,&value);
      Serial.print(" # values = ");
      Serial.println(numExtracted);
      if (numExtracted == 2) {
        if (channel == 16) {
          if (!value) {
            sleepDriver(true);
          } else {
            sleepDriver(false);
          }
        } else {
          if (driverSleep) {
            sleepDriver(false);
          }
          driver->pwm(channel, value);
        }
      }
      
      // Reset index
      index = 0;
      Serial.println("ready, enter 2 space separated numbers like 0 255 for channel brightness");
    } else {
      inData[index] = aChar;
      index++;
    }
  }
}

void sleepDriver(boolean newSleep) {
  if (newSleep) {
    if (driver != NULL) {
      tlcmanager.reset();
    }
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    driver = NULL;
  } else {
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    tlcmanager.reset();
    driver = &(tlcmanager[0]);
  }
  driverSleep = newSleep;
}
      
