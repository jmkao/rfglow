#include "rfglow_globals.h"

#include <TLC59116.h>

#pragma once

void initLEDs();
void ledTick();

void incrementHue();
void toggleAutocycle();
void incrementBrightness();

void setupFlashLED();

void setHSV(unsigned int h, unsigned int s, unsigned int v);
void setHSVRaw(unsigned int h, unsigned int s, unsigned int v, int newMa = -1);
void setRGBRaw(unsigned char r, unsigned char g, unsigned char b);

