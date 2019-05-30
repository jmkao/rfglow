#include "buttons.h"
#include "leds.h"
#include "mesh.h"

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
  target_color target = getCurrentTarget();
  sendCommandToMesh(target.h, target.s, target.v, target.ms);
}

void doubleClickAction() {
  DEBUG_PRINTLN("Double click");
  toggleAutocycle();
}

void longPressAction() {
  DEBUG_PRINTLN("Long press");
  incrementBrightness();
  target_color target = getCurrentTarget();
  sendCommandToMesh(target.h, target.s, target.v, target.ms);
}

void buttonTick() {
  button.tick();
}
