#include "root.h"


void RootComponent::init(){

    // TODO: load the configuration from eeprom !!!


    this->sHandler += this;// subscribe callback

    // Serial port for debugging purposes
    Serial.begin(115200);

    LittleFS.begin();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);// hint: low == led on

    pinMode(D2, INPUT);
    pinMode(D5, OUTPUT);
    digitalWrite(D5, HIGH);// hint: low == led on
  
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
    this->server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/webSocket_nc_Transmission.html", String(), false);
    });

    // Route for javascript file
    this->server.on("/exec_script.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/exec_script.js", "text/javascript");
    });

    // Route for css file
    this->server.on("/ex_styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/ex_styles.css", "text/css");
    });

    // start server
    this->server.begin();

    // stop serial
    Serial.flush();
    Serial.end();

}

void RootComponent::onLoop(){
     //delay(1);// delete???

    ws.cleanupClients();

    sHandler.processSerialTransmission();

    // temp:
    //digitalWrite(ledPin, ledState);

    if(digitalRead(D2) == LOW){
        digitalWrite(D5, LOW);
        digitalWrite(LED_BUILTIN, LOW);
    }
    else{
        digitalWrite(D5, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

void RootComponent::onSendComplete(unsigned int bytesTransferred){
    String message("Send-Operation complete. Bytes transferred: ");
    message += bytesTransferred;
    this->notifyUser(message);
}

void RootComponent::onReceptionComplete(const char* buffer, size_t size){
    String message("Receive-Operation complete. Bytes received: ");
    message += size;
    this->notifyUser(message);

    // TODO: send buffer to Web-page!
}

void RootComponent::handleWebSocketMessage(void *arg, uint8_t *data, size_t len){

    AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;

        // temp:
      //String message("Package received. Size: ");
      //message += len;
      //message += " | Header: ";
      //message += (char)data[0];
      //message += (char)data[1];
      //message += (char)data[2];
      //message += (char)data[3];
      //message += (char)data[4];
      //this->ws.textAll(message);

      if(data[1] == 'D'){
        // this is a data-transmission

        if(data[0] == '_'){
          // this a single package (not segmented)
          // so we can start the serial transmission directly

          // set global configuration
          this->sHandler.config(scBaudRate, scDatabits, scParity, scStoppbits);

          // remove the header
          this->sHandler.setTransmissionData(String((char*)data), true, 5);

          // send the data
          this->sHandler.sendData();
        }
        else if(data[0] == 's'){
          // this is the start of a multi-package-transmission
            this->sHandler.reset();
            sHandler.setTransmissionData(String((char*)data), true, 5);
            this->notifyClients("_Cnxt1E");// send next package command
        }
        else if(data[0] == 'x'){
            // this is a multi-package-transmission
            // so we must append the data to preliminary transmissions
            this->sHandler.addTransmissionData(String((char*)data), true, 5);
            // get package index
            String pIndex;
            if(data[2] != '0'){
                pIndex += (char)data[2];
            }
            if(data[3] != '0'){
                pIndex += (char)data[3];
            
            }
            pIndex += (char)data[4];
            auto inx = pIndex.toInt();
            inx++;

            String msg("_Cnxt");
            msg += inx;
            msg += "E";

            this->notifyClients(msg);// send next package command
        }
        else if(data[0] == 'e'){
          // this was the final package of a multi-package transmission
          // -> rebuild and open the serial connection
          this->sHandler.addTransmissionData(String((char*)data), true, 5);
          this->sHandler.config(scBaudRate, scDatabits, scParity, scStoppbits);
          this->sHandler.sendData();
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

void RootComponent::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {

        //getRootClass()->notifyClients(String("onEvent invoked!"));

    // NOTE: this is a static method, do not use the 'this' pointer, use the global instance instead!

    switch (type) {
      case WS_EVT_CONNECT:
        //getRootClass()->notifyClients(String("WS_EVT_CONNECT!"));
        getRootClass()->isConnected = true;
        getRootClass()->notifyClients("_Ccon");
        //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        //getRootClass()->notifyClients(String("WS_EVT_DISCONNECT!"));
        getRootClass()->isConnected = false;
        //Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        getRootClass()->handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
        getRootClass()->notifyClients(String("WS_EVT_PONG!"));
        //Serial.printf("Ping received from address: %s\n", client->remoteIP().toString().c_str());
        break;
      case WS_EVT_ERROR:
        getRootClass()->notifyClients(String("WS_EVT_ERROR!"));
        break;
  }
}

void RootComponent::notifyUser(String msg){
    String usermsg("_Cumg");
    usermsg += msg;
    this->ws.textAll(usermsg);
}