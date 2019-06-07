#include "buttons.h"
#include "leds.h"

OneButton button(BUTTON_PIN, true);

void initButtons() {
  // Init OneButton
  button.attachClick(clickAction);
  button.attachDoubleClick(doubleClickAction);
  button.attachLongPressStart(longPressAction);
  button.setClickTicks(150);
}

void clickAction() {
  DEBUG_PRINTLN("Single click");
  incrementHue();
}

void doubleClickAction() {
  DEBUG_PRINTLN("Double click");
  toggleAutocycle();
}

void longPressAction() {
  DEBUG_PRINTLN("Long press");
  incrementBrightness();
}

void buttonTick() {
  button.tick();
}
