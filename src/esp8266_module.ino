#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>

ESP8266WebServer server(80);
const char* host = "probatium-bt0001";
String dataJson = "";
String height = "0";
String weight = "0";

const size_t capacity = 2*JSON_OBJECT_SIZE(2);
DynamicJsonDocument doc(capacity);
JsonObject data = doc.createNestedObject("data");

void setup() {
  Serial.begin(57600);
  
  doc["version"] = "1.0";
  data["height"] = height;
  data["weight"] = weight;
  
  MDNS.begin(host);
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  if (!wifiManager.autoConnect("probatium.io")) {
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  
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
  MDNS.addService("http", "tcp", 80);

  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void loop() {
  getDataFromSerial();
  server.handleClient();
  MDNS.update();
}

void getDataFromSerial() {
  String medidas = "";
  while (Serial.available()) {
    char c = Serial.read();  //gets one byte from serial buffer
    medidas += c; //makes the String readString
    delay(2);  //slow looping to allow buffer to fill with next character
  }

  Serial.print("Data recibida del Arduino: ");
  Serial.println(medidas);

  // 170|85,34

  if (medidas.length() >0) {
    height = getValue(medidas, '|', 0);
    weight = getValue(medidas, '|', 1);

    createJSON();
  }
}

void createJSON() {
  data["height"] = height;
  data["weight"] = weight;
  serializeJsonPretty(doc, Serial);
  deserializeJson(doc, dataJson);
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
