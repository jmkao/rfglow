/***********************************************
 Global initializations
************************************************/
String log_prefix = String("RFG: ");

// TLC59116 Setup
TLC59116Manager tlcmanager;
TLC59116* driver = NULL;

// CC1101 Setup
boolean packetAvailable = false;

// Voltage indicator setup
volatile bool adcDone;

// OneButton Setup
OneButton button(BUTTON_PIN, true);

byte fhss_schema[] = FHSS_SCHEMA;
unsigned long last_rx_time = 0;
bool fhss_on = false;
unsigned long lastHopMicros = 0;
boolean isInPreHop = false;
byte ptr_fhss_schema = 0;
unsigned int packets_received = 0;

unsigned long lastCmdMs = 0;

unsigned int mH = 0;
unsigned int mS = 255;
unsigned int mB = 15;

unsigned int ma = 61;

int clickState = 0;

boolean isAutoCycle = false;
unsigned long prevMs = 0;

void setupFlashLED() {
  driver->pattern(0b1100000000000000);
  delay(50);
  driver->pattern(0b0000011000000000);
  delay(50);
  driver->pattern(0b0000000000011000);
  delay(50);
  driver->pattern(0b0000000000000000);

  driver->enable_outputs(false);  
}
