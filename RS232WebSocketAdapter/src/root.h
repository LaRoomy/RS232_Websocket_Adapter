#include <Arduino.h>

/*********
 Author: Hans Philipp Zimmermann
*********/

// Import required libraries
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "serial.h"

class RootComponent;

RootComponent* getRootClass();

class RootComponent 
    : public ISerialTransmissionEvents {

    public:
        RootComponent(){}
        ~RootComponent(){}

        void init();
        void onLoop();

        void onSendComplete(unsigned int bytesTransferred);
        void onReceptionComplete(const char* buffer, size_t size);

        //void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

        void notifyClients(String msg){
            this->ws.textAll(msg);
        }

        // this is temp: this is only a placeholder for the dynamic values implemented later

        // network credentials work
        //const char* ssid = "CT Workstation AP";
        //const char* password = "30321065";

        // network credentials home
        const char* ssid = "Delker Zimmi Net";
        const char* password = "60458561901103846208";


    private:
        AsyncWebServer server = AsyncWebServer(80);
        AsyncWebSocket ws = AsyncWebSocket("/ws");
        SerialTransmissionHandler sHandler;

        bool isConnected = false;

        // Default Serial configuration
        unsigned int scBaudRate = 115200;
        PARITY scParity = PARITY::NONE;
        DATABITS scDatabits = DATABITS::EIGHT;
        STOPPBITS scStoppbits = STOPPBITS::ONE;


        void initWebSocket() {
            this->ws.onEvent(RootComponent::onEvent);
            this->server.addHandler(&ws);
        }

        void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

        static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len);


        /*String processor(const String& var){
            return String();
        }*/

        void notifyUser(String msg);

};