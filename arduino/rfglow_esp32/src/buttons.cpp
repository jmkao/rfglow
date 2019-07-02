#include "buttons.h"
#include "leds.h"
#include "mesh.h"

OneButton button(BUTTON_PIN, true);

#define OFF 0, 0, 0
#define RED 360, 255, 120
#define BLUE 600, 255, 120
#define WHITE 360, 0, 60
#define ORANGE 375, 255, 120
#define GREEN 480, 255, 120
#define PURPLE 630, 200, 120
#define PINK 699, 220, 120
#define YELLOW 420, 255, 120
#define LGREEN 480, 200, 120
#define LBLUE 600, 200, 120
#define LPINK 699, 185, 120
#define VIOLET 640, 220, 120
#define LIME 470, 255, 120
#define HPINK 660, 255, 120
#define TURQUOISE 540, 255, 120

const target_color clickColors[][16] = {
  { {OFF, 0}, {RED, 0}, {BLUE, 0}, {WHITE, 0}, {ORANGE, 0}, {GREEN, 0}, {PURPLE, 0}, {PINK, 0}, {YELLOW, 0}, {LGREEN, 0}, {LBLUE, 0}, {LPINK, 0}, {VIOLET, 0}, {LIME, 0}, {HPINK, 0}, {TURQUOISE, 0}},
  { {OFF, 0}, {RED, 300}, {BLUE, 300}, {WHITE, 300}, {ORANGE, 300}, {GREEN, 300}, {PURPLE, 300}, {PINK, 300}, {YELLOW, 300}, {LGREEN, 300}, {LBLUE, 300}, {LPINK, 300}, {VIOLET, 300}, {LIME, 300}, {HPINK, 300}, {TURQUOISE, 300}}
};

uint8_t currentColorIndex = 0;
uint8_t currentSetIndex = 0;

void initButtons() {
  // Init OneButton
  button.attachClick(clickAction);
  button.attachDoubleClick(doubleClickAction);
  button.attachLongPressStart(longPressAction);
  button.setClickTicks(150);
}

void clickAction() {
  DEBUG_PRINTLN("Single click");
  currentColorIndex = (currentColorIndex + 1) % 16;
  changeTarget(clickColors[currentSetIndex][currentColorIndex]);
  sendCommandToMesh(clickColors[currentSetIndex][currentColorIndex]);
}

void doubleClickAction() {
  DEBUG_PRINTLN("Double click");
  target_color orig = getCurrentTarget();
  vbatLEDOn();
  delay(1000);
  changeTarget(orig);
}

void longPressAction() {
  DEBUG_PRINTLN("Long press");
  currentColorIndex = (currentColorIndex + 1) % 16;
  currentSetIndex = (currentSetIndex +1) % 2;
  changeTarget(clickColors[currentSetIndex][currentColorIndex]);
  sendCommandToMesh(clickColors[currentSetIndex][currentColorIndex]);
}

void buttonTick() {
  button.tick();
}
