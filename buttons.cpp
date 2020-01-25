#include "buttons.h"

void initButtons() {
    pinMode(DEFAULT_AUX_01_PIN, INPUT_PULLUP);
}
bool isPressed(uint8_t button) {
  switch(button) {
    case BUTTON_AUX_01:
      return !digitalRead(DEFAULT_AUX_01_PIN);
      break;
    default:
      return false;
      break;
    }
  return false;
}
