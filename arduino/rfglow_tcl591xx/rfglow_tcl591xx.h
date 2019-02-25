/***********************************************
 Tunable parameters
************************************************/
// Uncomment to enable debug logging
#define RFG_DEBUG

// Pin that the button is on
// Newer boards (after adding u.fl connector) are on pin 18 for eventual use with interrupts
// Older boards are on pin 19
#define BUTTON_PIN 18
//#define BUTTON_PIN 19

// Color change interval for button-controlled auto cycling
#define AUTO_INTERVAL 100

// FHSS params - must match what's in RF transmitter
#define hopIntervalMicros 40000L
#define preHopMicros 10000L
#define FHSS_SCHEMA { 9, 17, 15, 11, 13, 8, 2, 10, 18, 12, 5, 4, 6, 19, 14, 1, 16, 7, 3, 0 }

/***********************************************
 Fixed preprocs, don't change below
************************************************/
#define VBAT_ADC_PIN 0

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

// Log settings for TLC59116 library
#define TLC59116_LOWLEVEL 0
#define TLC59116_DEV 0

#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "HardwareSerial.h"

#include <OneButton.h>

#include <TLC59116.h>

#ifdef RFG_DEBUG
  #define DEBUG_PRINTLN(x) Serial.println (log_prefix + x)
  #define DEBUG_PRINT(x) Serial.print (log_prefix + x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif
