#include <Arduino.h>

/*********
 ????
*********/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// Replace with your network credentials
const char* ssid = "CT Workstation AP";
const char* password = "30321065";

bool ledState = 0;
const int ledPin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
    <head>
        <title>NC Program Remote Transmission</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <meta name="keywords" content="cnc, nc, rs232, websocket, transmission">
        <meta name="description" content="This site is a websocket connection to a rs232 interface">
        <meta name="Author" content="Hans Philipp Zimmermann">
        <meta charset="utf-8">
        <link rel="icon" href="data:,">
        <style>
body {
    background: #333333;
    color: #FFFFFF;
}

section {
    position: relative;
    width: 80%%;
    left: 50%%;
    transform: translate(-50%%, 0);
}
 
.centerHClass {
    position: relative;
    left: 50%%;
    transform: translateX(-50%%);
}
 
#headerSpan {
    text-align: center;
}
 
#nc_content_text_area {
    margin-top: 20px;
    resize: none;
    display: block;
    margin-left: auto;
    margin-right: auto;
    width: 90%%;
    height: 600px;
    max-width: 600px;
    color: white;
    background-color: #555555;
    font-family: Consolas, Menlo, Monaco, Lucida Console, Liberation Mono, DejaVu Sans Mono, Bitstream Vera Sans Mono, Courier New, monospace, serif;
    letter-spacing: 0.1em;
}
 
#send_receive_button_container {
    margin-top: 40px;
    display: inline-block;
}
 
#sendButton {
    margin-right: 30px;
}
 
.gButtonClass {
    padding: 10px 30px;
    min-width: 200px;
    font-size: 22px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #777777;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
}
 
.gButtonClass:active {
    background-color: #888888;
    box-shadow: 2 2px #CDCDCD;
    transform: translateY(2px);
  } 

#notificationAreaSpan {
    text-align: center;
}

#notificationView {
    margin-top: 50px;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    font-size: 20px;
    color: goldenrod;
}
        </style>
        <script type="text/javascript">
var gateway = `ws://${window.location.hostname}/ws`;

var webSocket;

function initWebSocket(){
    console.log("init WebSocket Session");
    webSocket = new WebSocket(gateway);
    webSocket.onopen = onWebSocketOpen;
    webSocket.onclose = onWebSocketClose;
    webSocket.onmessage = onWebSocketMessage;
}

function onWebSocketOpen(event){
    console.log("WebSocket opened");
}

function onWebSocketClose(event){
    console.log("WebSocket closed - try to re-init in 2000 milliseconds");
    setTimeout(initWebSocket, 2000);
}

function onWebSocketMessage(event){

    // temp ?!
    notifyUser(event.data);

}

function notifyUser(message){
    document.getElementById('notificationView').innerHTML = message
}

function initControls(){
    document.getElementById('sendButton').addEventListener('click', onSendButtonClicked);
    document.getElementById('receiveButton').addEventListener('click', onReceiveButtonClicked);
}

function onLoad(event){
    initWebSocket();
    initControls();
}

window.addEventListener('load', onLoad);

function onSendButtonClicked(){
    webSocket.send("Send");
    //notifyUser("This is a Message !")
}

function onReceiveButtonClicked(){
    webSocket.send("Receive");
}        
        </script>
        <link rel="icon" href="data:,">
    </head>
    <body>
        <section>
            <span id="headerSpan">
                <h2>RS232 Websocket Bridge</h2>
            </span>
            <span>
                <textarea id="nc_content_text_area" name="NCContentTextArea">place the nc content here</textarea>
            </span>
            <div id="send_receive_button_container" class="centerHClass">
                <button id="sendButton" class="gButtonClass">Send</button>
                <button id="receiveButton" class="gButtonClass">Receive</button>
            </div>
            <span id="notificationAreaSpan">
                <p id="notificationView">Notification</p>
            </span>
        </section>
    </body>
<html>
)rawliteral";

void notifyClients() {
  Serial.printf("notifyClients invoked ! Message: %s\n", String(ledState).c_str());
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {

  //Serial.printf("Incoming Websocket Message >>  %s\n", (char*)data);
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

      Serial.println((char*)data);

      String message((char*)data);
      message += " - was your data !";

      ws.textAll(message);

      if (strcmp((char*)data, "Send") == 0){
        ledState = 1;
      }
      if(strcmp((char*)data, "Receive") == 0){
        ledState = 0;
      }

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
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
        Serial.printf("Ping received from address: %s\n", client->remoteIP().toString().c_str());
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
  
  Serial.println("processor function invoked!");
  
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      Serial.println("... is ON");
      return "ON";
    }
    else{
      Serial.println("... is OFF");
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  

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
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}

void loop() {
  delay(1);
  ws.cleanupClients();
  digitalWrite(ledPin, ledState);
}
