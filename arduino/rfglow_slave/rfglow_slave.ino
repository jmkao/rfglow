#include <EEPROM.h>
#include "panstamp.h"
#include "cc1101.h"
#include "calibration.h"

// My Debug Setup

#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println (x)
  #define DEBUG_PRINT(x) Serial.print (x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif


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
//byte syncWord[] = {39,39};

#define RFCHANNEL 2

CC1101 cc1101;

boolean packetAvailable = false;

// Power setup #defines
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


void cc1101Interrupt(void) {
  packetAvailable = true;
  
//  CCPACKET packet;
//    
//  detachInterrupt(0);
//            
//  if(cc1101.receiveData(&packet) > 0) {
//    if (packet.crc_ok && packet.length > 1) {
//      processData(packet.data);
//    }
//  }
//
//  attachInterrupt(0, cc1101Interrupt, FALLING);
//
//  packetAvailable = false;
}

/**
 * setup
 *
 * Arduino setup function
 */
void setup()
{
 
  power_adc_disable();
  
  #ifdef DEBUG
    Serial.begin(38400);
  #endif
  DEBUG_PRINTLN("Setup started");

  // Init CC1101
  cc1101.init();
  //cc1101.reset();
  
  configRadio4800v();
  //configRadio1200f();

  // These are all set by writeReg() directly now
  //cc1101.setChannel(RFCHANNEL, true);
  //cc1101.setSyncWord(syncWord, true);
  //cc1101.setCarrierFreq(CFREQ_868);  
  //cc1101.setDevAddress(receiverAddress, true);
  
  cc1101.disableAddressCheck();
  
  // Calibrate internal RC oscillator
  //rcOscCalibrate();
  
  delay(100);
  
  cc1101.setRxState();
  
  attachInterrupt(0, cc1101Interrupt, FALLING);

  ShiftPWM.SetAmountOfRegisters(numRegisters);
  ShiftPWM.SetPinGrouping(1);
  ShiftPWM.Start(pwmFrequency, maxBrightness);
  ShiftPWM.SetAll(0);
  ShiftPWM.OneByOneFast();
  
  #ifdef DEBUG
    //ShiftPWM.PrintInterruptLoad();
  #endif
  DEBUG_PRINTLN(ShiftPWM.m_ledFrequency);
  DEBUG_PRINTLN(ShiftPWM.m_maxBrightness);
  DEBUG_PRINTLN("Setup complete");
  
}

/**
 * loop
 *
 * Arduino main loop
 */
void loop()
{
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

  DEBUG_PRINT("Received values: ");
  DEBUG_PRINT(hue);
  DEBUG_PRINT(", ");
  DEBUG_PRINT(saturation);
  DEBUG_PRINT(", ");
  DEBUG_PRINTLN(brightness);

  ShiftPWM.SetHSV(0, hue, saturation, brightness);
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

void configRadio1200f() {
  // 1.2kbps 7 byte variable, 58khz bw

cc1101.writeReg(CC1101_IOCFG2,0x2E);  //GDO2 Output Pin Configuration
cc1101.writeReg(CC1101_IOCFG1,0x2E);  //GDO1 Output Pin Configuration
cc1101.writeReg(CC1101_IOCFG0,0x01);  //GDO0 Output Pin Configuration
cc1101.writeReg(CC1101_FIFOTHR,0x0E); //RX FIFO and TX FIFO Thresholds
cc1101.writeReg(CC1101_SYNC1,0xD3);   //Sync Word, High Byte
cc1101.writeReg(CC1101_SYNC0,0x91);   //Sync Word, Low Byte
cc1101.writeReg(CC1101_PKTLEN,0xFF);  //Packet Length
cc1101.writeReg(CC1101_PKTCTRL1,0x04);//Packet Automation Control
cc1101.writeReg(CC1101_PKTCTRL0,0x05);//Packet Automation Control
cc1101.writeReg(CC1101_ADDR,0x00);    //Device Address
cc1101.writeReg(CC1101_CHANNR,0x00);  //Channel Number
cc1101.writeReg(CC1101_FSCTRL1,0x06); //Frequency Synthesizer Control
cc1101.writeReg(CC1101_FSCTRL0,0x00); //Frequency Synthesizer Control
cc1101.writeReg(CC1101_FREQ2,0x23);   //Frequency Control Word, High Byte
cc1101.writeReg(CC1101_FREQ1,0x31);   //Frequency Control Word, Middle Byte
cc1101.writeReg(CC1101_FREQ0,0x3B);   //Frequency Control Word, Low Byte
cc1101.writeReg(CC1101_MDMCFG4,0x7B); //Modem Configuration
cc1101.writeReg(CC1101_MDMCFG3,0x83); //Modem Configuration
cc1101.writeReg(CC1101_MDMCFG2,0x13); //Modem Configuration
cc1101.writeReg(CC1101_MDMCFG1,0x22); //Modem Configuration
cc1101.writeReg(CC1101_MDMCFG0,0xF8); //Modem Configuration
cc1101.writeReg(CC1101_DEVIATN,0x34); //Modem Deviation Setting
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
cc1101.writeReg(CC1101_WORCTRL,0xFB); //Wake On Radio Control
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

