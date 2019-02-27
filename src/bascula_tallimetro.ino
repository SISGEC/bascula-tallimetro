#include <NewPing.h>
#include <SoftwareSerial.h>

#define ULTRA_TRIGGER_PIN  12
#define ULTRA_ECHO_PIN     11
#define ULTRA_MAX_DISTANCE 30

#define ESP_SERIAL_RX 2
#define ESP_SERIAL_TX 3

String dataJSON;
int height = 0;
float weight = 0;
 
NewPing sonar(ULTRA_TRIGGER_PIN, ULTRA_ECHO_PIN, ULTRA_MAX_DISTANCE);
SoftwareSerial ESPserial(ESP_SERIAL_RX, ESP_SERIAL_TX);

void setup() {
  Serial.begin(115200);
  ESPserial.begin(57600);
}

void loop() {
  delay(50);
  setHeight();
  setWeight();
  
  createJSON();
  sendLineToESP(dataJSON);
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

void createJSON() {
  String j1 = "{\"version\":\"1.0\",\"height\":";
  String j2 = ", \"weight\":";
  String j3 = "}";
  dataJSON = j1 + height + j2 + weight + j3;
}

String readLineFromESP() {
  if (ESPserial.available()){
    return ESPserial.read();
  }
}

void sendLineToESP(String line) {
  if (ESPserial.available()){
    //int lsize = line.length();
    //char copy[lsize];
    //line.toCharArray(copy, lsize);
    Serial.print("Enviando datos a ESP: ");
    Serial.println(line);
    ESPserial.print(line);
  } else {
    Serial.println("ESP no disponible.");
  }
}
