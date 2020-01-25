/*
   Borrowed from: https://www.youtube.com/watch?v=k_D_Qu0cgu8
   And from: https://learn.sparkfun.com/tutorials/using-artnet-dmx-and-the-esp32-to-drive-pixels/all
   Requires: https://github.com/rstephan/ArtnetWifi

   Example to send via python: https://gist.github.com/fdemmer/7705019
   And this: https://raw.githubusercontent.com/hydronics2/python_artnet/master/udp_test
   // needs sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

*/
#include "network.h"
#include "webserver.h"
#include "artnethandler.h"
#include "buttons.h"

void setup() {
  Serial.println("Entering setup");
  Serial.begin(115200);
  initButtons();
  printSettings();

  // make ap mode activate if
  // a) cannot connect to network
  // b) button is held down on re-start
  if (isPressed(BUTTON_BOOT_SERVICE_MODE) || !initNetwork()) {
    Serial.println("Could not join wifi network. Starting AP Mode");
    startAPMode("test-ap","test-pass");
    Serial.println("Starting Web Server");
    initWebServer();
    return;
  }

  // TODO: test if web server and artnet listener can run nicely together
  // also check that web server is on the same core as artnet
  // initWebServer();
  startArtnetListener();

}

void loop() {
  // add a delay so the loop does not burn cpu with overhead
  // the internals run asynchronously to this thread
  delay(1000);
}
