#include "rfglow_globals.h"

#include <BLEDevice.h>
//#include <BLEUtils.h>
#include <BLEServer.h>

#pragma once

#define RX_CHAR_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"

void initBLE();
void stopBLE();
void startBLE();

boolean isBleConnected();
