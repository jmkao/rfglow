#include "rfglow_globals.h"
#include "ble.h"
#include "leds.h"

#pragma once

#include <painlessMesh.h>

void initMesh();

void meshTick();

void sendCommandToMesh(unsigned int tH, unsigned int tS, unsigned int tV, unsigned int tMS = 0);