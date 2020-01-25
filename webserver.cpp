#include "settings.h"
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char pageTemplate[] PROGMEM = R"=====(<!DOCTYPE html>
<html><head><meta content='width=device-width' name='viewport'><title>Update</title></head><style>
* {
  font-family: arial;
  }
input[type=text] {
    border-radius: .2rem;
    padding: .4rem;
    margin: .4rem .2rem;
    border: 1px solid #ccc;
}
input[type="submit"] {
    margin: .4rem auto;
    display: block;
    width: 80%%;
    background-color: lightskyblue;
    border: 1px solid aliceblue;
    border-radius: .4rem;
    padding: .5rem;
    cursor: pointer;
}
label {
    width: 5rem;
    display: inline-block;
    text-align: right;
    margin: .4rem 1rem;
}
</style>
<body>
%CONTENT%
</body></html>)=====";


String settingsFormProcessor(const String& var) {
  String settingsForm;
  settingsForm = "<form method='POST' action='/api/net'>";
  preferences.begin("net", false);
  settingsForm += "<label>ssid</label><input type='text' name='ssid' value='" + preferences.getString("ssid") + "'/><br/>";
  settingsForm += "<label>password</label><input type='text' name='password' value='" + preferences.getString("password") + "' /><br/>";
  settingsForm += "<input type='submit' value='Update Network Settings'>";
  preferences.end();
  settingsForm += "</form><br/>";
  settingsForm += "<form method='POST' action='/api/artnet'>";
  preferences.begin("artnet", false);
  settingsForm += "<label>LED count</label><input type='text' name='numLeds' value='" + String(preferences.getUInt("numLeds")) + "'/><br/>";
  settingsForm += "<label>Universe</label><input type='text' name='startUniverse' value='" + String(preferences.getUInt("startUniverse")) + "' /><br/>";
  preferences.end();
  settingsForm += "<input type='submit' value='Update Artnet Settings'>";
  settingsForm += "</form><br />";
  settingsForm += "<form method='GET' action='/api/reset'>";
  settingsForm += "<input type='submit' value='Reset to Defaults'>";
  settingsForm += "</form>";
  settingsForm += "<form method='GET' action='/api/reboot'>";
  settingsForm += "<input type='submit' value='Reboot'>";
  settingsForm += "</form>";
  return settingsForm;
}

//TODO don't require preferences be set in.. just create
//a new object locally
void printSettings() {
  Serial.println("===== settings =====");
  preferences.begin("net", false);
  Serial.print("SSID: ");
  Serial.println(preferences.getString("ssid"));
  Serial.print("PASS: ");
  Serial.println(preferences.getString("password"));
  preferences.end();

  preferences.begin("artnet", false);
  Serial.print("LED Count: ");
  Serial.println(preferences.getUInt("numLeds"));
  Serial.print("Universe: ");
  Serial.println(preferences.getUInt("startUniverse"));
  preferences.end();
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void initWebServer() {
  // Route for root / web page
  server.on("/", HTTP_GET, [&pageTemplate](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", pageTemplate, settingsFormProcessor);
  });

  server.on("/api/net", HTTP_GET, [&preferences](AsyncWebServerRequest *request){
    preferences.begin("net", false);
    String ssid = preferences.getString("ssid", DEFAULT_NET_SSID);
    preferences.end();

    String json = "{\"ssid\":\"" + ssid + "\"}";
    request->send_P(200, "application/json", json.c_str());
  });

  server.on("/api/net", HTTP_POST, [&preferences](AsyncWebServerRequest *request){
    preferences.begin("net");
    preferences.putString("ssid", request->arg("ssid"));
    preferences.putString("password", request->arg("password"));
    preferences.end();
    printSettings();
    request->redirect("/");
    // request->send_P(200, "text/html", "updated");
  });

  
  server.on("/api/artnet", HTTP_GET, [&preferences](AsyncWebServerRequest *request){
    preferences.begin("artnet", false);
    int numLeds = preferences.getUInt("numLeds", DEFAULT_NUM_LEDS);
    int startUniverse = preferences.getUInt("startUniverse", DEFAULT_START_UNIVERSE);
    preferences.end();
    String json = "{\"numLeds\":" + String(numLeds) + ", \"startUniverse\": " + String(startUniverse) + "}";
    request->send_P(200, "application/json", json.c_str());
  });

  server.on("/api/artnet", HTTP_POST, [&preferences](AsyncWebServerRequest *request){
    Serial.println("setting num leds");
    preferences.begin("artnet");
    preferences.putUInt("numLeds", request->arg("numLeds").toInt());
    preferences.putUInt("startUniverse", request->arg("startUniverse").toInt());
    preferences.end();
    printSettings();
    request->redirect("/");
    //    request->send_P(200, "text/html", "updated");
  });

  server.on("/api/reboot", HTTP_GET, [&preferences](AsyncWebServerRequest *request){
    Serial.println("Restarting in 10 seconds");
    request->send_P(200, "text/html", "Restarting in 10 seconds");
    delay(10000);
    ESP.restart();
  });

  server.on("/api/reset", HTTP_GET, [&preferences](AsyncWebServerRequest *request){
    Serial.println("Resetting to factory defaults, then rebooting");
    request->send_P(200, "text/html", "Resetting to factory defaults, then rebooting");
    delay(10000);
    preferences.begin("artnet");
    preferences.clear();
    preferences.end();
    preferences.begin("net");
    preferences.clear();
    preferences.end();

    ESP.restart();
  });

  server.onNotFound(notFound);
 
  server.begin();

}
