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
  isAutoCycle = false;
  mH = (mH + 60) % 360;
  setHSV(mH, mS, mB);
}

void doubleClickAction() {
  DEBUG_PRINTLN("Double click");
  isAutoCycle = !isAutoCycle;
}

void longPressAction() {
  DEBUG_PRINTLN("Long press");
  mB = (mB + 60) % 240;
  setHSV(mH, mS, mB);
}

void buttonTick() {
  button.tick();
}
