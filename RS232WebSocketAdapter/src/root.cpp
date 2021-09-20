#include "root.h"




void RootComponent::init(){

    // TODO: load the configuration from eeprom !!!


    this->sHandler += this;// subscribe serial transmission callback

    // Serial port for debugging purposes
    Serial.begin(115200);

    LittleFS.begin();

    EEPROM.begin(EE_PROM_SIZE);

    this->loadPersistentData();

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

    // Route for config / web page
    this->server.on("/webSocket_nc_Configuration.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/webSocket_nc_Configuration.html", String(), false);
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

          // TODO: handle all config messages!

          switch(data[5]){
            case 'B':
              // this is a baudrate config command
              this->onBaudrateConfigTransmission((char*)data);
              break;
            case 'D':
              // this is a databit config command
              this->onDatabitConfigTransmission((char*)data);
              break;
            case 'P':
              // this is a parity config command
              this->onParityConfigTransmission((char*)data);
              break;
            case 'S':
              // this is a stoppbit config command
              this->onStoppbitConfigTransmission((char*)data);
              break;
            default:
              break;
          }

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

void RootComponent::loadPersistentData(){

  uint8_t val;

  val = EEPROM.read(EE_BAUD_ADDR);
  if(val != 255){
      this->scBaudRate = this->baudIndexToBaudValue(val);
  }
  val = EEPROM.read(EE_DATAB_ADDR);
  if(val != 255){
    if(val == 5)this->scDatabits = DATABITS::FIVE;
    if(val == 6)this->scDatabits = DATABITS::SIX;
    if(val == 7)this->scDatabits = DATABITS::SEVEN;
    if(val == 8)this->scDatabits = DATABITS::EIGHT;
  }
  val = EEPROM.read(EE_PARITY_ADDR);
  if(val != 255){
    if(val == 0)this->scParity = PARITY::NONE;
    if(val == 1)this->scParity = PARITY::EVEN;
    if(val == 2)this->scParity = PARITY::ODD;
  }
  val = EEPROM.read(EE_STOPPB_ADDR);
  if(val != 255){
    if(val == 1)this->scStoppbits = STOPPBITS::ONE;
    if(val == 2)this->scStoppbits = STOPPBITS::TWO;
  }
}

void RootComponent::notifyUser(String msg){
    String usermsg("_Cumg");
    usermsg += msg;
    this->ws.textAll(usermsg);
}

void RootComponent::onBaudrateConfigTransmission(const char* data){

  String baud;

  for(unsigned int i = 6; i < 12; i++){
    if(data[i] == 'E'){
      break;
    }
    else {
      baud += (char)data[i];
    }
  }

  notifyUser(baud);

  auto bIndex =
    this->baudIndexFromBaudValue(
      baud.toInt()
      );

    if(bIndex == 255){
      this->notifyUser("Error invalid baud value (invalid index)");
    }
    else {
      this->scBaudRate = baud.toInt();

      EEPROM.write(EE_BAUD_ADDR, bIndex);

      EEPROM.commit()
        ? this->notifyUser("Baudrate successful saved.")
        : this->notifyUser("Error saving Baudrate.");
    }
}

void RootComponent::onDatabitConfigTransmission(const char* data){
  if(data[6] == '5'){
    // 5 databits
    this->scDatabits = DATABITS::FIVE;

    EEPROM.write(EE_DATAB_ADDR, 5);

    if(EEPROM.commit()){
      this->notifyUser(DATABITS_SAVE_SUCCESS_MESSAGE);
    }
    else {
      this->notifyUser(DATABITS_SAVE_FAILED_MESSAGE);
    }
  }
  else if(data[6] == '6'){
    // 6 databits
    this->scDatabits = DATABITS::SIX;

    EEPROM.write(EE_DATAB_ADDR, 6);

    if(EEPROM.commit()){
      this->notifyUser(DATABITS_SAVE_SUCCESS_MESSAGE);
    }
    else {
      this->notifyUser(DATABITS_SAVE_FAILED_MESSAGE);
    }
  }
  else if(data[6] == '7'){
    // 7 databits
    this->scDatabits = DATABITS::SEVEN;

    EEPROM.write(EE_DATAB_ADDR, 7);

    if(EEPROM.commit()){
      this->notifyUser(DATABITS_SAVE_SUCCESS_MESSAGE);
    }
    else {
      this->notifyUser(DATABITS_SAVE_FAILED_MESSAGE);
    }
  }
  else if(data[6] == '8'){
    // 8 databits
    this->scDatabits = DATABITS::EIGHT;

    EEPROM.write(EE_DATAB_ADDR, 8);

    EEPROM.commit()
      ? this->notifyUser(DATABITS_SAVE_SUCCESS_MESSAGE)
      : this->notifyUser(DATABITS_SAVE_FAILED_MESSAGE);

      // TODO: apply this syntax everywhere (if it works!)
  }
}

void RootComponent::onParityConfigTransmission(const char* data){
  
  if(data[6] == 'n'){
    // parity: none
    this->scParity = PARITY::NONE;

    EEPROM.write(EE_PARITY_ADDR, 0);
    
    if(EEPROM.commit()){
      this->notifyUser("Parity successful saved.");
    }
    else {
      this->notifyUser("Error occured while saving parity.");
    }
  }
  else if(data[6] == 'e'){
    // parity even
    this->scParity = PARITY::EVEN;

    EEPROM.write(EE_PARITY_ADDR, 1);

    if(EEPROM.commit()){
      this->notifyUser("Parity successful saved.");
    }
    else {
      this->notifyUser("Error occured while saving parity.");
    }
  }
  else if(data[6] == 'o'){
    // parity odd
    this->scParity = PARITY::ODD;

    EEPROM.write(EE_PARITY_ADDR, 2);

    if(EEPROM.commit()){
      this->notifyUser("Parity successful saved.");
    }
    else {
      this->notifyUser("Error occured while saving parity.");
    }
  }
  else {
    this->notifyUser("Invalid parity data");
  }
}

void RootComponent::onStoppbitConfigTransmission(const char* data){
  if(data[6] == '1'){
    // one stoppbit
    this->scStoppbits = STOPPBITS::ONE;

    EEPROM.write(EE_STOPPB_ADDR, 1);

    if(EEPROM.commit()) {
      this->notifyUser("Stoppbits successful saved.");
    }
    else {
      this->notifyUser("Error saving stoppbits.");
    }
  }
  else if(data[6] == '2'){
    // two stoppbits
    this->scStoppbits = STOPPBITS::TWO;
    
    EEPROM.write(EE_STOPPB_ADDR, 2);

    if(EEPROM.commit()) {
      this->notifyUser("Stoppbits successful saved.");
    }
    else {
      this->notifyUser("Error saving stoppbits.");
    }
  }
}

uint8_t RootComponent::baudIndexFromBaudValue(long baudVal){
  switch (baudVal){
    case 75:
      return 0;
    case 300:
      return 1;
    case 1200:
      return 2;
    case 2400:
      return 3;
    case 4800:
      return 4;
    case 9600:
      return 5;
    case 14400:
      return 6;
    case 19200:
      return 7;
    case 28800:
      return 8;
    case 38400:
      return 9;
    case 57600:
      return 10;
    case 115200:
      return 11;
    case 230400:
      return 12;
    case 460800:
      return 13;
    default:
      return 255;
  }
}

long RootComponent::baudIndexToBaudValue(uint8_t index){
   switch (index){
    case 0:
      return 75;
    case 1:
      return 300;
    case 2:
      return 1200;
    case 3:
      return 2400;
    case 4:
      return 4800;
    case 5:
      return 9600;
    case 6:
      return 14400;
    case 7:
      return 19200;
    case 8:
      return 28800;
    case 9:
      return 38400;
    case 10:
      return 57600;
    case 11:
      return 115200;
    case 12:
      return 230400;
    case 13:
      return 460800;
    default:
      return 0;
  }

}