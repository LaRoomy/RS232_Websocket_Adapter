#include "root.h"




void RootComponent::init(){

    uint8_t ledState = 1;

    this->sHandler += this;// subscribe serial transmission callback

    // Serial port for debugging purposes
    Serial.begin(115200);// TEMP !!!!!!!!!!!!!

    LittleFS.begin();

    EEPROM.begin(EE_PROM_SIZE);

    //EEPROM.write(EE_WLAN_PASSWRD_LENGTH_ADDR, 0);
    //EEPROM.write(EE_WLAN_SSID_LENGTH_ADDR, 0);
    //EEPROM.commit();

    this->loadPersistentData();
    this->sHandler.setAutoDetectEndOfTransmission(this->autoDetectEOT);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);// hint: low == led on

    pinMode(D2, INPUT);
    pinMode(D5, OUTPUT);
    digitalWrite(D5, ledState);// hint: low == led on
  
    // Connect to Wi-Fi (if credentials are set)
    if((this->network_ssid.length() > 0) && (this->network_password.length() > 0)){

      Serial.println(this->network_ssid.c_str());
      Serial.write('\n');
      Serial.println(this->network_password.c_str());   // this is NOT CORRECT only 604585619!!!

      WiFi.begin(
        this->network_ssid.c_str(),
        this->network_password.c_str()
        );

      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
        ledState = !ledState;
        digitalWrite(D5, ledState);
      }
          
      // Print ESP Local IP Address
      Serial.println(WiFi.localIP());
      // set led to on, to indicate connection status: connected
      digitalWrite(LED_BUILTIN, LOW);

      // activate multicast dns
      if(!MDNS.begin("ncinterface")){
        Serial.println("Error setting up MDNS!");
      }
      else {
        Serial.println("MDNS started.");
      }

      // init websocket
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

      // disable config-led for further usage
      digitalWrite(D5, LOW);

      // stop serial
      Serial.flush();
      Serial.end();
    }
    else {
      Serial.println("No network credentials. Only config mode available.");
      // stop serial
      Serial.flush();
      Serial.end();

      // set config mode
      this->mode = DEVICEMODE::CONFIGMODE;

      // indicate entering the config-mode by led
      digitalWrite(D5, LOW);

      // config and start serial interface
      this->sHandler.config(this->scBaudRate, this->scDatabits, this->scParity, this->scStoppbits);
      this->sHandler.startTerminal();
    }
}

void RootComponent::onLoop(){

    if(this->mode == DEVICEMODE::SOCKETMODE){
      // websocket
      ws.cleanupClients();
      // mdns
      MDNS.update();
      // handle serial transmission
      sHandler.processSerialTransmission();

      // check system switch for config-mode
      if(digitalRead(D2) == LOW){
        // enter config-mode
        this->enterConfigMode();
      }
      //else{
      //    digitalWrite(D5, HIGH);
      //}

      // indicate connection status with system led
      if(WiFi.isConnected()){
        digitalWrite(LED_BUILTIN, LOW);
      }
      else {
        // wifi is disconnected -> try to re-connect
        digitalWrite(LED_BUILTIN, HIGH);
        this->reConnect();
      }
    }
    else {
      // this is the config mode
      this->sHandler.processTerminalTransmissions();
      // check system switch to exit config-mode
      if(digitalRead(D2) == HIGH){
        // leave the config mode
        this->leaveConfigMode();
      }
    }
}

void RootComponent::onSendComplete(unsigned int bytesTransferred){
    String message(I_MSG_SEND_OPERATION_COMPLETE);
    message += bytesTransferred;
    this->notifyUser(message);
}

void RootComponent::onReceptionComplete(const String& buffer, size_t size){
    if(this->mode == DEVICEMODE::CONFIGMODE){
      this->onHandleTerminalCommunication(buffer);
    }
    else {
      // TEMP TEMP TEMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //this->enterConfigMode();

      // notify user
      String message(I_MSG_RECEIVE_OPERATION_COMPLETE);
      message += size;
      this->notifyUser(message);
      // send buffer to websocket
      this->ws.textAll("_Csrt");
      String setter("_Crrr");
      setter += buffer;
      this->ws.textAll(setter);
    }
}

void RootComponent::onReceptionStarted(){
    ws.textAll("_CsrsE");
}

void RootComponent::onError(TRANSMISSION_ERROR error){
  switch(error){
    case TRANSMISSION_ERROR::TRANSMISSION_ALREADY_IN_PROGRESS:
      this->notifyUser(E_MSG_TRANSMISSION_ALREADY_IN_PROGRESS);
      break;
    case TRANSMISSION_ERROR::INVALID_SEND_IS_ACTIVE:
      this->notifyUser(E_MSG_INVALID_SEND_IS_ACTIVE);
      break;
    default:
      break;
  }
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
            case 'G':
              // this is a configuration request
              this->onConfigurationRequest();
              break;
            case 'R':
              // this is a reset command
              this->onResetCommand();
              break;
            case 'X':
              // this is a receive command
              this->onReceiveCommand();
              break;
            case 'M':
              // this is a stop-reception command
              this->onStopReception();
              break;
            case 'A':
              // this is a auto-detect config command
              this->onAutoDetectConfigTransmission((char*)data);
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

void RootComponent::enterConfigMode(){

  // set config mode
  this->mode = DEVICEMODE::CONFIGMODE;

  // indicate entering the config-mode by led
  digitalWrite(D5, LOW);

  // close websocket
  this->ws.closeAll();
  this->server.end();

  // stop mdns
  MDNS.end();

  // disconnect
  WiFi.disconnect();

  // switch off wifi status led
  digitalWrite(LED_BUILTIN, HIGH);

  // config and start serial interface
  this->sHandler.config(this->scBaudRate, this->scDatabits, this->scParity, this->scStoppbits);
  this->sHandler.startTerminal();
}

void RootComponent::leaveConfigMode(){

  // stop terminal transmission mode
  this->sHandler.exitTerminal();

  // set normal modes
  this->mode = DEVICEMODE::SOCKETMODE;
  this->inputMode = INPUTMODE::NONE;

  // indicate leaving the config mode with led
  digitalWrite(D5, HIGH);

  // try to re-connect
  this->reConnect();
}

void RootComponent::reConnect(){

  //ESP.restart();

  // TODO!
}

void RootComponent::loadPersistentData(){

  uint8_t val;

  // baudrate
  val = EEPROM.read(EE_BAUD_ADDR);
  if(val != 255){
      this->scBaudRate = this->baudIndexToBaudValue(val);
  }
  // databits
  val = EEPROM.read(EE_DATAB_ADDR);
  if(val != 255){
    if(val == 5)this->scDatabits = DATABITS::FIVE;
    if(val == 6)this->scDatabits = DATABITS::SIX;
    if(val == 7)this->scDatabits = DATABITS::SEVEN;
    if(val == 8)this->scDatabits = DATABITS::EIGHT;
  }
  // parity
  val = EEPROM.read(EE_PARITY_ADDR);
  if(val != 255){
    if(val == 0)this->scParity = PARITY::NONE;
    if(val == 1)this->scParity = PARITY::EVEN;
    if(val == 2)this->scParity = PARITY::ODD;
  }
  // stoppbits
  val = EEPROM.read(EE_STOPPB_ADDR);
  if(val != 255){
    if(val == 1)this->scStoppbits = STOPPBITS::ONE;
    if(val == 2)this->scStoppbits = STOPPBITS::TWO;
  }
  // auto-detect eot
  val = EEPROM.read(EE_AD_EOT_ADDR);
  if(val != 255){
    if(val)this->autoDetectEOT = true;
    else this->autoDetectEOT = false;
  }
  // ssid
  val = EEPROM.read(EE_WLAN_SSID_LENGTH_ADDR);
  if((val < 255)&&(val > 0)){

    Serial.printf("SSID length is: %i", val);
    
    for(unsigned int i = EE_WLAN_SSID_START_ADDR; i < ((unsigned int)(EE_WLAN_SSID_START_ADDR + val)); i++){
      this->network_ssid += (char)EEPROM.read(i);
    }

    Serial.printf("SSID is: %s", this->network_ssid.c_str());

  }
  // password
  val = EEPROM.read(EE_WLAN_PASSWRD_LENGTH_ADDR);
  if((val < 255)&&(val > 0)){

    Serial.printf("PASSWORD length is: %i", val);

    for(unsigned int i = EE_WLAN_PASSWRD_START_ADDR; i < ((unsigned int)(EE_WLAN_PASSWRD_START_ADDR + val)); i++){
      this->network_password += (char)EEPROM.read(i);
    }
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

  auto bIndex =
    this->baudIndexFromBaudValue(
      baud.toInt()
      );

    if(bIndex == INVALID_BAUD_INDEX){
      this->notifyUser(E_MSG_INVALID_BAUD_VALUE);
    }
    else {
      this->scBaudRate = baud.toInt();

      EEPROM.write(EE_BAUD_ADDR, bIndex);

      EEPROM.commit()
        ? this->notifyUser(I_MSG_BAUDRATE_SAVE_SUCCESS)
        : this->notifyUser(E_MSG_BAUDRATE_SAVE_FAILED);
    }
}

void RootComponent::onDatabitConfigTransmission(const char* data){

  bool executeCommit = false;

  if(data[6] == '5'){
    // 5 databits
    this->scDatabits = DATABITS::FIVE;
    EEPROM.write(EE_DATAB_ADDR, 5);
    executeCommit = true;
  }
  else if(data[6] == '6'){
    // 6 databits
    this->scDatabits = DATABITS::SIX;
    EEPROM.write(EE_DATAB_ADDR, 6);
    executeCommit = true;
  }
  else if(data[6] == '7'){
    // 7 databits
    this->scDatabits = DATABITS::SEVEN;
    EEPROM.write(EE_DATAB_ADDR, 7);
    executeCommit = true;

  }
  else if(data[6] == '8'){
    // 8 databits
    this->scDatabits = DATABITS::EIGHT;
    EEPROM.write(EE_DATAB_ADDR, 8);
    executeCommit = true;
  }
  else {
    this->notifyUser(E_MSG_INVALID_DATA);
  }

  if(executeCommit){
      EEPROM.commit()
      ? this->notifyUser(I_MSG_DATABITS_SAVE_SUCCESS)
      : this->notifyUser(E_MSG_DATABITS_SAVE_FAILED);  
  }
}

void RootComponent::onParityConfigTransmission(const char* data){

  bool executeCommit = false;
  
  if(data[6] == 'n'){
    // parity: none
    this->scParity = PARITY::NONE;
    EEPROM.write(EE_PARITY_ADDR, 0);
    executeCommit = true;
  }
  else if(data[6] == 'e'){
    // parity even
    this->scParity = PARITY::EVEN;
    EEPROM.write(EE_PARITY_ADDR, 1);
    executeCommit = true;
  }
  else if(data[6] == 'o'){
    // parity odd
    this->scParity = PARITY::ODD;
    EEPROM.write(EE_PARITY_ADDR, 2);
    executeCommit = true;
  }
  else {
    this->notifyUser(E_MSG_INVALID_DATA);
  }

  if(executeCommit){
    EEPROM.commit()
    ? this->notifyUser(I_MSG_PARITY_SAVE_SUCCESS)
    : this->notifyUser(E_MSG_PARITY_SAVE_FAILED);
  }
}

void RootComponent::onStoppbitConfigTransmission(const char* data){

  bool executeCommit = false;

  if(data[6] == '1'){
    // one stoppbit
    this->scStoppbits = STOPPBITS::ONE;
    EEPROM.write(EE_STOPPB_ADDR, 1);
    executeCommit = true;
  }
  else if(data[6] == '2'){
    // two stoppbits
    this->scStoppbits = STOPPBITS::TWO;
    EEPROM.write(EE_STOPPB_ADDR, 2);
    executeCommit = true;
  }
  else {
    this->notifyUser(E_MSG_INVALID_DATA);
  }

  if(executeCommit){
    EEPROM.commit()
    ? this->notifyUser(I_MSG_STOPPBITS_SAVE_SUCCESS)
    : this->notifyUser(E_MSG_STOPPBITS_SAVE_FAILED);
  }
}

void RootComponent::onAutoDetectConfigTransmission(const char* data){

  data[6] == '1'
    ? EEPROM.write(EE_AD_EOT_ADDR, 1)
    : EEPROM.write(EE_AD_EOT_ADDR, 0);
  
  data[6] == '1'
    ? this->autoDetectEOT = true
    : this->autoDetectEOT = false;

  this->sHandler.setAutoDetectEndOfTransmission(this->autoDetectEOT);
  
  EEPROM.commit()
    ? this->notifyUser(I_MSG_AUTODETECT_SAVE_SUCCESS)
    : this->notifyUser(E_MSG_AUTODETECT_SAVE_FAILED);
}

void RootComponent::onReceiveCommand(){
    this->sHandler.config(this->scBaudRate, this->scDatabits, this->scParity, this->scStoppbits);
    this->sHandler.startReceiving();
}

void RootComponent::onStopReception(){
  this->sHandler.stopReceiving();
}

void RootComponent::onConfigurationRequest(){
  String configurationString("_Ccrq");

  // syntax
  // '_Ccrq' + [baudindex_10][baudindex_1] + [databits] + [parity] + [stoppbits] + [autodetect] + 'E'

  auto baudIndex = this->baudIndexFromBaudValue(this->scBaudRate);
  if(baudIndex < 10){
    configurationString += '0';
  }
  configurationString += baudIndex;
  configurationString += (uint8_t)this->scDatabits;
  configurationString += (uint8_t)this->scParity;
  configurationString += (uint8_t)this->scStoppbits;
  configurationString += this->autoDetectEOT ? 1 : 0;
  configurationString += 'E';

  this->ws.textAll(configurationString);
}

void RootComponent::onResetCommand(){
    this->scBaudRate = 9600;
    this->scDatabits = DATABITS::EIGHT;
    this->scParity = PARITY::NONE;
    this->scStoppbits = STOPPBITS::ONE;

    EEPROM.write(EE_BAUD_ADDR, this->baudIndexFromBaudValue(this->scBaudRate));
    EEPROM.write(EE_DATAB_ADDR, (uint8_t)this->scDatabits);
    EEPROM.write(EE_PARITY_ADDR, (uint8_t)this->scParity);
    EEPROM.write(EE_STOPPB_ADDR, (uint8_t)this->scStoppbits);

    if(EEPROM.commit()){
      this->notifyUser(I_MSG_CONFIG_RESET_SUCCESS);
    }
    else {
      this->notifyUser(E_MSG_CONFIG_RESET_ERROR);
    }
    this->onConfigurationRequest();
}

void RootComponent::onHandleTerminalCommunication(const String& data){
  if(data.length() > 0){
    if(this->inputMode == INPUTMODE::SSID_MODE){
      // save ssid
      if(data.length() > 255){
        this->sHandler.terminal_sendData(E_MSG_MAXIMUM_CHAR_EXCEEDED);
      }
      else {
        // set
        this->network_ssid = data;
        // save
        for(unsigned int i = EE_WLAN_SSID_START_ADDR; i < (EE_WLAN_SSID_START_ADDR + data.length()); i++){
          EEPROM.write(
            i,
            data.charAt(i - EE_WLAN_SSID_START_ADDR)
            );
        }
        EEPROM.write(EE_WLAN_SSID_LENGTH_ADDR, (uint8_t)data.length());
        EEPROM.commit()
          ? this->sHandler.terminal_sendData(I_MSG_SSID_SAVE_SUCCESS)
          : this->sHandler.terminal_sendData(E_MSG_SSID_SAVE_FAILED);
      }
      // normalize
      this->inputMode = INPUTMODE::NONE;
    }
    else if(this->inputMode == INPUTMODE::PASSWORD_MODE){
      // save password
      if(data.length() > 255){
        this->sHandler.terminal_sendData(E_MSG_MAXIMUM_CHAR_EXCEEDED);
      }
      else {
        // set
        this->network_password = data;
        // save
        for(unsigned int i = EE_WLAN_PASSWRD_START_ADDR; i < (EE_WLAN_PASSWRD_START_ADDR + data.length()); i++){
            EEPROM.write(
              i,
              data.charAt(i - EE_WLAN_PASSWRD_START_ADDR)
            );
        }
        EEPROM.write(EE_WLAN_PASSWRD_LENGTH_ADDR, (uint8_t)data.length());
        EEPROM.commit()
          ? this->sHandler.terminal_sendData(I_MSG_PASSWORD_SAVE_SUCCESS)
          : this->sHandler.terminal_sendData(E_MSG_PASSWORD_SAVE_FAILED);
      }
      // normalize
      this->inputMode = INPUTMODE::NONE;
    }
    else {
      if(data.charAt(0) == '?'){
        // help request
        this->sHandler.terminal_sendData(TERM_OUTGOING_HELP_RESPONSE_1);
        delay(20);
        this->sHandler.terminal_sendData(TERM_OUTGOING_HELP_RESPONSE_2);
        delay(20);
        this->sHandler.terminal_sendData(TERM_OUTGOING_HELP_RESPONSE_3);
      }
      else {

    String temp("mode is: ");
    temp += ((unsigned int)this->inputMode);
    temp += " | data is: ";
    temp += data;


    // temp!!!!!!!!!
    sHandler.terminal_sendData(temp);


        if(data == "exit"){
          // exit the config-mode? or do it only if the hardware switch is switched?
        }
        else if(data == TERM_INCOMING_SSID_CONFIG_REQUEST){
          this->sHandler.terminal_sendData(TERM_OUTGOING_SSID_ENTER_PROMPT);
          this->inputMode = INPUTMODE::SSID_MODE;
        }
        else if(data == TERM_INCOMING_PASSWORD_CONFIG_REQUEST){
          this->sHandler.terminal_sendData(TERM_OUTGOING_PASSWORD_ENTER_PROMPT);
          this->inputMode = INPUTMODE::PASSWORD_MODE;
        }
      }
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
      return INVALID_BAUD_INDEX;
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