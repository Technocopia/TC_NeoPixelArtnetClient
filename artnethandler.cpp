#include "artnethandler.h"
#include <ArtnetWifi.h>
#include <WiFiUdp.h>
#include <FastLED.h>
#include "settings.h"
const byte ledDataPin = 2;
bool debug = 0;

// the universe this device is configured to start on
// if there are fewer than CHANNELS_PER_UNIVERSE channels, this will
// be the start and end
uint16_t startUniverse;
int numLeds;

CRGB* leds;
const int channelsPerLed = DEFAULT_CHANNELS_PER_LED;
int numberOfChannels;

/*
   When numLeds > 512 we are going to have to span multiple universes
   The reciever will wait until all universes are received before triggering
   output
*/
int universeCount;

// track universes received so we know when to flip the sendFrame flag
bool* universesReceived;
bool sendFrame = 0;

// metrics
int fps = 0;
int frameCount = 0;
unsigned long lastCheck = micros();
int checkEvery = 100;

ArtnetWifi artnet;
TaskHandle_t ArtnetListenerTask, LedDriverTask;
SemaphoreHandle_t baton;

bool validUniverse(uint16_t universe) {
  return universe >= startUniverse && universe < (startUniverse + universeCount);
}

void clearFrameState() {
  memset(universesReceived, 0, universeCount);
  sendFrame = 0;
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  if (debug) {
    Serial.println("DMX FRAME");
    Serial.println("sequence");
    Serial.println(sequence);
    Serial.println("universe");
    Serial.print(universe);
    Serial.print("/");
    Serial.println(universeCount);
    Serial.println("length");
    Serial.println(length);
    Serial.println("-----------------------");
    Serial.print("Frame: ");
    Serial.print(frameCount);
    Serial.print(", ");
    Serial.print(fps);
    Serial.println(" fps");
    Serial.println("-----------------------");
  }

  if (!validUniverse(universe)) {
    if (debug) {
      Serial.println("Ignoring Universe, out of range");
    }
    return;
  }

  universesReceived[universe - startUniverse] = 1;

  bool frameComplete = 1;
  for (int i = 0 ; i < universeCount ; i++)
  {
    if (universesReceived[i] == 0)
    {
      frameComplete = 0;
      break;
    }
  }

  xSemaphoreTake( baton, portMAX_DELAY );

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / channelsPerLed; i++)
  {
    int led = i + (universe - startUniverse) * (CHANNELS_PER_UNIVERSE / channelsPerLed);
    if (led < numLeds) {
      // not supported w/ fastled :(
      //if (channelsPerLed == 4)
      //  leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2],  data[i * 3 + 2]);
      if (channelsPerLed == 3)
        leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }

  // update the shared flag so the led task
  // knows that there is something new to output
  sendFrame = frameComplete;

  // release baton so it can be written out to strip
  xSemaphoreGive( baton );
}

//int readCount = 0;
void artnetListener( void * params)
{
  // set up the udp stuff
  Serial.println("starting artnet listner");
  artnet.setArtDmxCallback(onDmxFrame);
  artnet.begin();

  while (true) {
    artnet.read();
  }
}

void ledDriver( void * params )
{
  while (true) {
    if ( sendFrame ) {
      xSemaphoreTake( baton, portMAX_DELAY );
      //pipe out the led stuff
      FastLED.show();
      clearFrameState();

      if (debug) {
        frameCount++;
        if (!(frameCount % checkEvery)) {
          unsigned long currentTime = micros();
          fps = (checkEvery * 1000000) / (currentTime - lastCheck );
          Serial.print(fps);
          Serial.println(" fps");
          Serial.print(uxTaskGetStackHighWaterMark( NULL ));
          Serial.println(" watermark");
          Serial.println("========================================================================================");

          lastCheck = currentTime;
        }
      }

      xSemaphoreGive( baton );
    }
    else {
      // if not sending a frame, breath a little so we don't trigger the watchdog thing
      // delay(50);
    }
    delay(10);
  }
}

void doWelcomeRoutine()
{
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void initLeds() {
  preferences.begin("artnet");
  startUniverse = preferences.getUInt("startUniverse", DEFAULT_START_UNIVERSE);
  numLeds = preferences.getUInt("numLeds", DEFAULT_NUM_LEDS);
  preferences.end();
  numberOfChannels = numLeds * channelsPerLed;
  universeCount = numberOfChannels / CHANNELS_PER_UNIVERSE + ((numberOfChannels % CHANNELS_PER_UNIVERSE) ? 1 : 0);
  leds = new CRGB[numLeds];
  universesReceived = new bool[universeCount];
  FastLED.addLeds<WS2812, ledDataPin, GRB>(leds, numLeds);
  doWelcomeRoutine();
}

void startArtnetListener() {
  initLeds();

  Serial.println("Beginning Artnet listener");
  baton = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    artnetListener,
    "artnetListener",
    2048, //hmmm 1k too small, causes stack frame issues
    NULL,
    1,
    &ArtnetListenerTask,
    1);

  delay(500);  // needed to start-up task1

  xTaskCreatePinnedToCore(
    ledDriver,
    "ledDriver",
    2048,  //hmmm 1k too small, causes stack frame issues
    baton,
    1,
    &LedDriverTask,
    0);
}
