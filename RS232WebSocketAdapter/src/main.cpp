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

// Replace with your network credentials
const char* ssid = "CT Workstation AP";
const char* password = "30321065";

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

  //Serial.printf("Incoming Websocket Message >>  %s\n", (char*)data);
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

      //Serial.println((char*)data);

      String strData((char*)data);
      //message += " - was your data !";

      String message("Sending data. Size: ");
      message += strData.length();
      message += " Bytes";


      ws.textAll(message);

      if (strcmp((char*)data, "Send") == 0){
        ledState = !ledState;
      }
      else {

          Serial.begin(2400, SERIAL_7E1);
          Serial.print((char*)data);
          //Serial.end();

          //Serial.
          
      }

      //if(strcmp((char*)data, "Receive") == 0){
      //  ledState = 0;
      //}

    
      

    //if (strcmp((char*)data, "toggle") == 0) {
    //  Serial.println("led-state changed");
    //  ledState = !ledState;
    //  notifyClients();
    //}
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
  Serial.end();
}

void loop() {
  delay(1);

  ws.cleanupClients();

  sHandler.processSerialTransmission();

  // temp:
  digitalWrite(ledPin, ledState);
}
