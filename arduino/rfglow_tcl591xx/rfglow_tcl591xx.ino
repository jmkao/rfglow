#include <avr/pgmspace.h>
#include <HardwareSerial.h>

#include <OneButton.h>

#include <cc1101.h>
#include <panstamp.h>

//#define TLC59116_LOWLEVEL 1
//#define TLC59116_DEV 1
#include <TLC59116.h>

// Logging Setup
#define DEBUG

String log_prefix = String("RFG: ");

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println (log_prefix + x)
  #define DEBUG_PRINT(x) Serial.print (log_prefix + x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

// TLC59116 Setup
TLC59116Manager tlcmanager;
TLC59116* driver = NULL;

// CC1101 Setup
#define RFCHANNEL 2
byte networkAddress[] = {39,0};
byte receiverAddress = 5;
//byte syncWord[] = {39,39};
CC1101 cc1101;
boolean packetAvailable = false;

// Power saving #defines
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

// OneButton Setup
OneButton button(4, true);
//ClickButton button(4, LOW, CLICKBTN_PULLUP);

void cc1101Interrupt(void) {
  packetAvailable = true;
}

void setup() {
  
  power_adc_disable();
  //power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  

  
  #ifdef DEBUG
    Serial.begin(38400);
  #endif
  DEBUG_PRINTLN("TCL591XX Test");
  DEBUG_PRINTLN("setup()");

  // Init CC1101
  cc1101.setTxPowerAmp(0x03);
  cc1101.init();
  configRadio();
  cc1101.disableAddressCheck();
  cc1101.setRxState();
  attachInterrupt(0, cc1101Interrupt, FALLING);


  // Init OneButton
  button.attachClick(clickAction);
  button.attachDoubleClick(doubleClickAction);
  button.attachLongPressStart(longPressAction);
  /*
  button.debounceTime = 20;
  button.multiclickTime = 85;
  button.longClickTime = 1000;
  */

  // Init TLC59116
  tlcmanager.init();
  
  //sleepDriver(false);
  driver = &(tlcmanager[0]);
  driver->enable_outputs(true);
  
  
  driver->pattern(0b1100000000000000);
  delay(50);
  driver->pattern(0b0000011000000000);
  delay(50);
  driver->pattern(0b0000000000011000);
  delay(50);
  driver->pattern(0b0000000000000000);

  driver->enable_outputs(false);  

}

#define RF_LOCKOUT_MS 2500
unsigned long lastCmdMs = 0;

unsigned int mH = 0;
unsigned int mS = 255;
unsigned int mB = 60;

int clickState = 0;

#define AUTO_INTERVAL 100
boolean isAutoCycle = false;
unsigned long prevMs = 0;

void loop() {
  
  unsigned long curMs = millis();
  
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

  if (curMs - lastCmdMs > RF_LOCKOUT_MS) {
    button.tick();
    if (isAutoCycle) {
      if (curMs - prevMs > AUTO_INTERVAL) {
        mH = (mH + 60) % 360;
        setHSV(mH, mS, mB);
        prevMs = curMs;
      }
    } else {
      prevMs = curMs;
    } 
  }
}

void clickAction() {
  isAutoCycle = false;
  mH = (mH + 60) % 360;
  setHSV(mH, mS, mB);
}

void doubleClickAction() {
  isAutoCycle = !isAutoCycle;
}

void longPressAction() {
  mB = (mB + 60) % 240;
  setHSV(mH, mS, mB);
}

void processData(byte *data) {

  unsigned int hue;
  unsigned int saturation;
  unsigned int brightness;

  hue = ((data[1] << 8) + data[2]);
  saturation = data[3];
  brightness = data[4];

  DEBUG_PRINT("Received values: ");
  DEBUG_PRINT(hue);
  DEBUG_PRINT(", ");
  DEBUG_PRINT(saturation);
  DEBUG_PRINT(", ");
  DEBUG_PRINTLN(brightness);

  setHSV(hue, saturation, brightness);
  
  lastCmdMs = millis();
}

void setHSV(unsigned int h, unsigned int s, unsigned int v) {
  DEBUG_PRINTLN("setHSV() called: "+h+" "+s+" "+v);
  setHSVRaw(h,s,v);
}

void setHSVRaw(unsigned int h, unsigned int s, unsigned int v) {
  
  DEBUG_PRINTLN("setHSVRaw() called: "+h+" "+s+" "+v);
  
  unsigned char r, g, b;
  unsigned char region, remainder, p, q, t;

  if (s == 0) {
    r = v;
    g = v;
    b = v;
  } else {
    region = h / 60;
    remainder = (h - (region * 60)) * 6; 

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
      case 0:
          r = v; g = t; b = p;
          break;
      case 1:
          r = q; g = v; b = p;
          break;
      case 2:
          r = p; g = v; b = t;
          break;
      case 3:
          r = p; g = q; b = v;
          break;
      case 4:
          r = t; g = p; b = v;
          break;
      default:
          r = v; g = p; b = q;
          break;
    }
  }
  setRGBRaw(r, g, b);
}

void setRGBRaw(unsigned char r, unsigned char g, unsigned char b) {
  DEBUG_PRINTLN("setRGBRaw() called: "+r+" "+g+" "+b);
  
  if (r==0 & g==0 & b==0) {
    driver->pattern(0x0000);
    driver->enable_outputs(false);
  } else {
    if (!driver->is_enabled()) {
      driver->enable_outputs(true);
    }
    
    byte leds[16];
    
    for (int i=0;i<5;i++) {
      // Set red channels
      leds[i] = r;
    }
    
    for (int i=5;i<10;i++) {
      // Set green channels
      leds[i] = g;
    }
    
    for (int i=11;i<16;i++) {
      // Set blue channels;
      leds[i] = b;
    }
    
    driver->set_outputs(leds);
  }
}

void configRadio() {
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

/*
// Serial test version
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
            driver->enable_outputs(false);
          } else {

          }
        } else {
          driver->enable_outputs(true);
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
*/

