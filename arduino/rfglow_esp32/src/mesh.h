#include "rfglow_globals.h"

#pragma once

void initMesh();

void meshTick();
boolean isMeshMasterPresent();

void sendCommandToMesh(target_color target, bool useSeq = false);
void sendCommandToMesh(unsigned int tH, unsigned int tS, unsigned int tV, unsigned int tMS = 0, bool useSeq = false);