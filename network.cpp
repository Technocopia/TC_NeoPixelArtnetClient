#include "network.h"
#include <WiFi.h>

/* define external variables */
String ssid;
String password;

String translateEncryptionType(wifi_auth_mode_t encryptionType);

String translateEncryptionType(wifi_auth_mode_t encryptionType) {

  switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
  }
}

void scanNetworks() {
  // potential hack
  WiFi.mode(WIFI_STA);

  int numberOfNetworks = WiFi.scanNetworks();

  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);

  if (numberOfNetworks == WIFI_SCAN_FAILED) {
    Serial.println("scanNetworks failed");
    return;
  }

  for (int i = 0; i < numberOfNetworks; i++) {
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));
    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("-----------------------");
  }
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(const char* ssid, const char* password)
{
  boolean state = true;
  int i = 0;

  // potential hack to make things work
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  if(NULL == ssid) {
    Serial.println("No ssid given. Checking config.");
  }

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi: ");
  Serial.print(ssid);
  Serial.print(" using password: ");
  Serial.println(password);

  // Wait for connection
  Serial.print("Connecting");

  //switch to this 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    //Serial.print("Hostname: ");
    //Serial.println(WiFi.hostname());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

boolean startAPMode(const char* ssid, const char* password) {
  WiFi.disconnect(true);
  Serial.println("");
  Serial.println("Starting AP Mode");
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(ssid, password);
  String mac = WiFi.macAddress();
  Serial.println("Mac Address: " + mac);
  Serial.print("Starting AP: ");
  Serial.print(ssid);
  Serial.print(":");
  Serial.print(password);
  Serial.println("");

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

bool initNetwork() {
  Serial.println("Initializing network");
  preferences.begin("net", false);
  ssid = preferences.getString("ssid", DEFAULT_NET_SSID);
  password = preferences.getString("password", DEFAULT_NET_PASS);
  preferences.end();

  if (!connectWifi(ssid.c_str(), password.c_str())) {
    Serial.println("Could not connect to network");
    return false;
  }
  return true;
}
