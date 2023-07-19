#include <ESP8266WiFi.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#include <Hash.h>

#ifndef STASSID
//#define STASSID "MiFibra-2FC9"
//#define STAPSK "jtYP6iaj"
#define STASSID "MUEVETE3"
#define STAPSK "formacion"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
SocketIOclient socketIO;

#define USE_SERIAL Serial

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      USE_SERIAL.printf("[IOc] Disconnected!\n");
      break;
    case sIOtype_CONNECT:
      USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);
      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      sendClientName();
      break;
    case sIOtype_EVENT:
      USE_SERIAL.printf("[IOc] get event: %s\n", payload);
      
         getOpen(payload, length);
      
      break;
    case sIOtype_ACK:
      USE_SERIAL.printf("[IOc] get ack: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_ERROR:
      USE_SERIAL.printf("[IOc] get error: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_EVENT:
      USE_SERIAL.printf("[IOc] get binary: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_ACK:
      USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
      hexdump(payload, length);
      break;
  }
}

void setup() {
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  // USE_SERIAL.begin(921600);
  USE_SERIAL.begin(115200);

  //Serial.setDebugOutput(true);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  // server address, port and URL
  //socketIO.begin("192.168.1.131", 3000, "/socket.io/?EIO=4");
  socketIO.begin("172.104.205.29", 3000, "/socket.io/?EIO=4");
  // event handler
  socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;
void loop() {
  socketIO.loop();
}

void sendClientName() {
  // creat JSON message for Socket.IO (event)
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // add evnet name
  // Hint: socket.on('event_name', ....
  array.add("storeClientInfo");

  // add payload (parameters) for the event
  JsonObject param1 = array.createNestedObject();
  param1["name"] = "rele1";

  // JSON to String (serializion)
  String output;
  serializeJson(doc, output);

  // Send event
  socketIO.sendEVENT(output);

  // Print JSON for debugging
  USE_SERIAL.println(output);
}

void getOpen(uint8_t * payload,size_t length){
          DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
          USE_SERIAL.print(F("deserializeJson() failed: "));
          USE_SERIAL.println(error.c_str()); 

          
          return;
        }
        String eventName = doc[0];
        int stat = doc[1];
        if (eventName == "open") {
          if (stat == 1) {
            digitalWrite(0, LOW);
            Serial.println("opennn..");
            delay(1000);
            digitalWrite(0, HIGH);
            Serial.println("close..");
          }
        }
}
