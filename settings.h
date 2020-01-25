#ifndef SETTINGS_H
#define SETTINGS_H
#include <Arduino.h>
#include <Preferences.h>

#define CHANNELS_PER_UNIVERSE 512
#define DEFAULT_START_UNIVERSE 1
#define DEFAULT_NUM_LEDS 64
/* this uses the fastled library
 * so far there is no rgbw support
 * so changing this to 4 won't work
 * 
 */
#define DEFAULT_CHANNELS_PER_LED 3

#define DEFAULT_NET_SSID ""
#define DEFAULT_NET_PASS ""

extern Preferences preferences;

#endif
