#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// http://forum.arduino.cc/index.php?topic=396450

ESP8266WebServer server(80);
const char* host = "probatium-bt0001";
String dataJson = "{\"version\":\"1.0\",\"height\":0, \"weight\":0}";

void setup() {
    Serial.begin(57600);
    MDNS.begin(host);
    WiFiManager wifiManager;
    //wifiManager.resetSettings();
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    if (!wifiManager.autoConnect("probatium.io")) {
      delay(3000);
      ESP.reset();
      delay(5000);
    }

    server.on("/data.json", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "application/json", dataJson);
    });
    server.on("/clear-data", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      //wifiManager.resetSettings();
      ESP.reset();
      server.send(200, "application/json", "{\"code\":\"OK\"}");
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);
}

void loop() {
  getDataFromSerial();
  server.handleClient();
  MDNS.update();
}

void getDataFromSerial() {
  if (Serial.available()) {
    Serial.println(dataJSON);
    dataJson = Serial.readString();
  }
}
