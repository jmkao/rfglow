#include "rfglow_globals.h"

#pragma once

#include <painlessMesh.h>

void initMesh();
void reinitMeshAPOnly();

void meshTick();
boolean isMeshMasterPresent();

void sendCommandToMesh(target_color target);
void sendCommandToMesh(unsigned int tH, unsigned int tS, unsigned int tV, unsigned int tMS = 0);