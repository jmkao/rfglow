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


#include <EEPROM.h>
#include "cc1101.h"
//#include "regtable.h"

// ShiftPWM Setup
#define SHIFTPWM_NOSPI
const int ShiftPWM_latchPin = 9;
const int ShiftPWM_clockPin = 8;
const int ShiftPWM_dataPin = 7;

const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;
#include <ShiftPWM.h>

unsigned char maxBrightness = 16;
unsigned int pwmFrequency = 1200;
int numRegisters = 1;

// CC1101 Setup
byte networkAddress[] = {39,0};
byte receiverAddress = 5;
byte syncWord[] = {39,39};

#define RFCHANNEL 2

CC1101 cc1101;

boolean packetAvailable = false;

void cc1101Interrupt(void) {
  packetAvailable = true;
  
  CCPACKET packet;
    
  detachInterrupt(0);
            
  if(cc1101.receiveData(&packet) > 0) {
    if (packet.crc_ok && packet.length > 1) {
      processData(packet.data);
    }
  }

  attachInterrupt(0, cc1101Interrupt, FALLING);

  packetAvailable = false;
}

/**
 * setup
 *
 * Arduino setup function
 */
void setup()
{
  
  power_adc_disable();
  
  Serial.begin(38400);
  Serial.println("Setup started");

  // Init CC1101
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


  cc1101.setChannel(RFCHANNEL, true);
  cc1101.setSyncWord(syncWord, true);
  
  cc1101.setDevAddress(receiverAddress, true);
  cc1101.disableAddressCheck();
  cc1101.setRxState();
  
  attachInterrupt(0, cc1101Interrupt, FALLING);

  ShiftPWM.SetAmountOfRegisters(numRegisters);
  ShiftPWM.SetPinGrouping(1);
  ShiftPWM.Start(pwmFrequency, maxBrightness);
  ShiftPWM.SetAll(0);
  ShiftPWM.OneByOneFast();
  //ShiftPWM.PrintInterruptLoad();
  Serial.println(ShiftPWM.m_ledFrequency);
  Serial.println(ShiftPWM.m_maxBrightness);
  Serial.println("Setup complete");
  
}

/**
 * loop
 *
 * Arduino main loop
 */
void loop()
{
  /*
  if(packetAvailable) {
    packetAvailable = false;
    CCPACKET packet;
    
    detachInterrupt(0);
    
    if(cc1101.receiveData(&packet) > 0) {
      if (packet.crc_ok && packet.length > 1) {
        processData(packet.data);
      }
    }
    
    attachInterrupt(0, cc1101Interrupt, FALLING);
  }
  */
}

void processData(byte *data) {

  unsigned int hue;
  unsigned int saturation;
  unsigned int brightness;

  hue = ((data[1] << 8) + data[2]);
  saturation = data[3];
  brightness = data[4];
  
/*
  hue = ((data[1] << 8) + data[2]);
  saturation = ((data[3] << 8) + data[4]);
  brightness = ((data[5] << 8) + data[6]);
*/
/*
  Serial.print("Received values: ");
  Serial.print(hue);
  Serial.print(", ");
  Serial.print(saturation);
  Serial.print(", ");
  Serial.println(brightness);
  */
  ShiftPWM.SetHSV(0, hue, saturation, brightness);
}
