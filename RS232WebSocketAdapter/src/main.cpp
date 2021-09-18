#include <Arduino.h>

/*********
 ????
*********/

// Import required libraries
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "serial.h"

// network credentials work
//const char* ssid = "CT Workstation AP";
//const char* password = "30321065";

// network credentials home
const char* ssid = "Delker Zimmi Net";
const char* password = "60458561901103846208";

// Global Serial configuration
unsigned int scBaudRate = 115200;
PARITY scParity = PARITY::NONE;
DATABITS scDatabits = DATABITS::EIGHT;
STOPPBITS scStoppbits = STOPPBITS::ONE;

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Create Processorclass for the serial transmission
SerialTransmissionHandler sHandler;

void notifyClients() {
  //Serial.printf("notifyClients invoked ! Message: %s\n", String(ledState).c_str());
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

      // temp:
      String message("Package received. Size: ");
      message += len;
      message += " | Header: ";
      message += (char)data[0];
      message += (char)data[1];
      message += (char)data[2];
      message += (char)data[3];
      message += (char)data[4];
      ws.textAll(message);

      if(data[1] == 'D'){
        // this is a data-transmission

        if(data[0] == '_'){
          // this a single package (not segmented)
          // so we can start the serial transmission directly

          // set global configuration
          sHandler.config(scBaudRate, scDatabits, scParity, scStoppbits);

          // remove the header
          sHandler.setTransmissionData(String((char*)data), true, 5);

          // send the data
          sHandler.sendData();
        }
        else if(data[0] == 's'){
          // this is the start of a multi-package-transmission
          sHandler.reset();
          sHandler.setTransmissionData(String((char*)data), true, 5);
        }
        else if(data[0] == 'x'){
          // this is a multi-package-transmission
          // so we must append the data to preliminary transmissions
          sHandler.addTransmissionData(String((char*)data), true, 5);
        }
        else if(data[0] == 'e'){
          // this was the final package of a multi-package transmission
          // -> rebuild and open the serial connection
          sHandler.addTransmissionData(String((char*)data), true, 5);
          sHandler.config(scBaudRate, scDatabits, scParity, scStoppbits);
          sHandler.sendData();
        }
      }
      else if(data[1] == 'C'){
        // this is a config/command transmission

        if(data[0] == '_'){ 

          // TODO: handle the config messages!

        } 
        else {
          // the config transmission doesn't exceed the max-single transmission size
          // so we don't handle the other header types
        }
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        //Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
        //Serial.printf("Ping received from address: %s\n", client->remoteIP().toString().c_str());
        break;
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  
  //Serial.println("processor function invoked!");
  
  //Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      //Serial.println("... is ON");
      return "ON";
    }
    else{
      //Serial.println("... is OFF");
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  LittleFS.begin();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(D2, INPUT);
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/webSocket_nc_Transmission.html", String(), false, processor);
  });

  // Route for javascript file
  server.on("/exec_script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/exec_script.js", "text/javascript");
  });

  // Route for css file
  server.on("/ex_styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/ex_styles.css", "text/css");
  });

  // start server
  server.begin();

  // stop serial
  Serial.flush();
  Serial.end();
}

void loop() {
  delay(1);

  ws.cleanupClients();

  sHandler.processSerialTransmission();

  // temp:
  digitalWrite(ledPin, ledState);

  if(digitalRead(D2) == LOW){
    digitalWrite(D5, HIGH);
  }
  else{
    digitalWrite(D5, LOW);
  }
}
