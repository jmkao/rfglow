// 2 includes!
#include <Wire.h>
#include "TLC59116.h"

// Need a manager
TLC59116Manager tlcmanager; 

void setup() {
    tlcmanager.init(); // inits Wire, devices, and collects them

    // You could do some initial patterns/setup here
    tlcmanager.broadcast().set(0, HIGH); // every device, channel 0 on
    }

void loop() {
    // blink the i'th channel of the i'th device
    for(int i=0; i<tlcmanager.device_count(); i++) { // tlcmanager knows how many
        tlcmanager[i].set(i, HIGH);
        delay(200);
        tlcmanager[i].set(i, LOW);
        delay(200);
        }
    }



