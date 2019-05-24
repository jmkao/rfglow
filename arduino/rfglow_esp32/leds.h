#include "rfglow_globals.h"

#include <TLC59116.h>

#pragma once

void initLEDs();
void ledTick();

void incrementHue();
void toggleAutocycle();
void stopAutocycle();
void incrementBrightness();

void setupFlashLED();

void fadeTo(unsigned int h, unsigned int s, unsigned int v, unsigned int ms);
void setHSV(unsigned int h, unsigned int s, unsigned int v);
void setHSVRaw(unsigned int h, unsigned int s, unsigned int v, int maLevel);
void setRGBRaw(unsigned char r, unsigned char g, unsigned char b);

