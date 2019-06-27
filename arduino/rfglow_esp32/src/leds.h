#include "rfglow_globals.h"

#include <TLC59116.h>

#pragma once

void initLEDs();
void ledTick();

void toggleAutocycle();
void stopAutocycle();

void setupFlashLED();

void changeTarget(target_color target);
void fadeLedTo(unsigned int h, unsigned int s, unsigned int v, unsigned int ms);
void switchLedTo(unsigned int h, unsigned int s, unsigned int v);

//void setHSV(unsigned int h, unsigned int s, unsigned int v);
void setHSVRaw(unsigned int h, unsigned int s, unsigned int v, int maLevel);
void setRGBRaw(unsigned char r, unsigned char g, unsigned char b);

target_color getCurrentTarget();