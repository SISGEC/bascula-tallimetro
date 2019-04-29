#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <Wire.h>
#include <NewPing.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define ULTRA_TRIGGER_PIN  12
#define ULTRA_ECHO_PIN     11
#define ULTRA_MAX_DISTANCE 30

#define SERIAL_BAUDIOS_RATE 57600
#define SERVER_PORT 80

#define WIFI_MANAGER_IP IPAddress(10,0,1,1)
#define WIFI_MANAGER_MASK IPAddress(255,255,255,0)
#define WIFI_MANAGER_HOST_NAME "probatium.io"

String dataJSON;
int height = 0;
float weight = 0;

const char* host = "probatium-bt0001";
String dataJson = "";
String height = "0";
String weight = "0";

const size_t capacity = 2*JSON_OBJECT_SIZE(2);
DynamicJsonDocument doc(capacity);
JsonObject data = doc.createNestedObject("data");

NewPing sonar(ULTRA_TRIGGER_PIN, ULTRA_ECHO_PIN, ULTRA_MAX_DISTANCE);
ESP8266WebServer server(SERVER_PORT);

void setup() {
  Serial.begin(SERIAL_BAUDIOS_RATE);
  setupESP();
}

void loop() {
  delay(50);
  setHeight();
  setWeight();
  createJSON();
  server.handleClient();
  MDNS.update();
}

void setHeight() {
  int distance = sonar.ping_cm();
  if(distance > ULTRA_MAX_DISTANCE) {
    distance = distance - ULTRA_MAX_DISTANCE;
  }
  if(distance < 0) {
    distance = 0;
  }
  height = ULTRA_MAX_DISTANCE - distance;
}

void setWeight() {
  weight = weight + 1;
}

void setupWifiManager() {
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.setAPStaticIPConfig(WIFI_MANAGER_IP, WIFI_MANAGER_IP, WIFI_MANAGER_MASK);
  if (!wifiManager.autoConnect(WIFI_MANAGER_HOST_NAME)) {
    delay(3000);
    ESP.reset();
    delay(5000);
  }
}

void setupServer() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", "Ready!");
  });
  
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
}

void setupESP() {
  doc["version"] = "1.0";
  data["height"] = height;
  data["weight"] = weight;
  
  MDNS.begin(host);
  setupWifiManager();
  setupServer();
  MDNS.addService("http", "tcp", 80);

  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void createJSON() {
  data["height"] = height;
  data["weight"] = weight;
  serializeJson(doc, dataJson);
}
