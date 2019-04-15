#include "rfglow_globals.h"

/*** Global Variable Initialization ***/
volatile bool adcDone = false;

unsigned long lastCmdMs = 0;

unsigned int mH = 0;
unsigned int mS = 255;
unsigned int mB = 15;

unsigned int ma = 61;

boolean isAutoCycle = false;
unsigned long prevMs = 0;
