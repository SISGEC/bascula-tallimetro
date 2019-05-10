#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>       //https://github.com/tzapu/WiFiManager
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <NewPingESP8266.h>
#include <HX711.h>
#include <ArduinoJson.h>

#define ULTRA_TRIGGER_PIN  D5
#define ULTRA_ECHO_PIN     D6
#define ULTRA_MAX_DISTANCE 200 // 2 metros
#define ULTRA_MIN_DISTANCE 100 // 1 metro

#define HX711_DOUT_PIN D3
#define HX711_PD_SCK_PIN A0
#define HX711_MAX_WEIGHT 180 // 180 kilos
#define HX711_MIN_WEIGHT 2 // 2 kilos
#define HX711_SCALE 439430.25
#define HX711_TARE_QTY 20

#define LCD_SDA_PIN D2
#define LCD_SCL_PIN D1

#define SERIAL_BAUDIOS_RATE 115200
#define SERVER_PORT 80

#define WIFI_MANAGER_IP IPAddress(10,0,1,1)
#define WIFI_MANAGER_MASK IPAddress(255,255,255,0)
#define WIFI_MANAGER_SSID "probatium.io"

String dataJSON;
int height = 0;
float weight = 0;

const char* host = "probatium-bt0001";
String dataJson = "";

const size_t capacity = 2*JSON_OBJECT_SIZE(2);
DynamicJsonDocument doc(capacity);
JsonObject data = doc.createNestedObject("data");

NewPingESP8266 sonar(ULTRA_TRIGGER_PIN, ULTRA_ECHO_PIN, ULTRA_MAX_DISTANCE);
HX711 balanza;
LiquidCrystal_I2C lcd(0x1B,16,2);
ESP8266WebServer server(SERVER_PORT);
WiFiManager wifiManager;

void setup() {
  Serial.begin(SERIAL_BAUDIOS_RATE);
  Serial.println("Test");
  setupHX711();
  setupLCD();
  setupESP();
  Serial.println("Probatium Ready!");
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
  Serial.print("Altura: ");
  Serial.println(distance);
  if(distance > ULTRA_MAX_DISTANCE) {
    distance = distance - ULTRA_MAX_DISTANCE;
  }
  if(distance < ULTRA_MIN_DISTANCE) {
    distance = 0;
  }
  height = ULTRA_MAX_DISTANCE - distance;
}

void setWeight() {
  Serial.print("Peso: ");
  if (balanza.is_ready()) {
    int result = balanza.get_units(20);
    Serial.println(result);
    if(result > HX711_MAX_WEIGHT) {
      result = HX711_MAX_WEIGHT;
    }
    if(result < HX711_MIN_WEIGHT) {
      result = 0;
    }
    weight = result;
  } else {
    Serial.println("Balanza not ready!");
  }
}

void setupHX711() {
  Serial.println("Setup HX711...");
  balanza.begin(HX711_DOUT_PIN, HX711_PD_SCK_PIN);
  balanza.set_scale(HX711_SCALE);
  balanza.tare(HX711_TARE_QTY);
}

void setupLCD() {
  Serial.println("Setup LCD...");
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.begin();
  lcd.backlight();
  lcd.home();
  lcd.print("Inicializando...");
  lcd.print(" PROBATIUM v2.0 ");
}

void setupWifiManager() {
  Serial.println("Setup WIFI Manager...");
  //wifiManager.resetSettings();
  wifiManager.setAPStaticIPConfig(WIFI_MANAGER_IP, WIFI_MANAGER_IP, WIFI_MANAGER_MASK);
  if (!wifiManager.autoConnect(WIFI_MANAGER_SSID)) {
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

  Serial.println("Setup ESP...");
  MDNS.begin(host);
  setupWifiManager();
  setupServer();
  MDNS.addService("http", "tcp", 80);

  String ip = WiFi.localIP().toString();

  Serial.println("");
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.println("");

  printLCD("IP:", ip);
  delay(5000);
}

String prepareLine(String line) {
  if(line.length() < 16) {
    for(int i = 0; i < 16; i++) {
      line = line + " ";
    }
  }
  return line;
}

void printLine(String line, int n) {
  if(n < 2) {
    lcd.home();
  } else {
    lcd.setCursor(0, 1);
  }
  line = prepareLine(line);
  lcd.print(line);
}

void printLCD(String line1, String line2) {
  lcd.backlight();
  if(line1.length() > 0) {
    printLine(line1, 1);
  }
  if(line1.length() > 0) {
    printLine(line2, 2);
  }
}

void createJSON() {
  data["height"] = height;
  data["weight"] = weight;
  dataJson = "";
  serializeJson(doc, dataJson);
  String heightText = "Altura: " + String(height);
  String weightText = "Peso: " + String(weight);
  printLCD(heightText, weightText);
}
