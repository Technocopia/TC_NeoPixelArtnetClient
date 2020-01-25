#ifndef BUTTONS_H
#define BUTTONS_H
#include <Arduino.h>
#define DEFAULT_AUX_01_PIN 15
#define BUTTON_AUX_01 1
#define BUTTON_BOOT_SERVICE_MODE BUTTON_AUX_01

void initButtons();
bool isPressed(uint8_t button);

#endif
