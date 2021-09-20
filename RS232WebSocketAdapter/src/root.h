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

#include <EEPROM.h>
#define     EE_BAUD_ADDR                0x00
#define     EE_DATAB_ADDR               0x01
#define     EE_PARITY_ADDR              0x02
#define     EE_STOPPB_ADDR              0x03
// free (for future parameter)
#define     EE_WLAN_SSID_LENGTH_ADDR    0x08
#define     EE_WLAN_PASSWRD_LENGTH_ADDR 0x09
#define     EE_WLAN_SSID_START_ADDR     0x0A
#define     EE_WLAN_SSID_MAX_ADDR       0x1FE
#define     EE_WLAN_PASSWRD_START_ADDR  0x1FF
#define     EE_WLAN_PASSWRD_MAX_ADDR    0x3FF

#define     EE_PROM_SIZE                 518

#include "stringTable.h"

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

        void loadPersistentData();

        void notifyUser(String msg);

        void onBaudrateConfigTransmission(const char* data);
        void onDatabitConfigTransmission(const char* data);
        void onParityConfigTransmission(const char* data);
        void onStoppbitConfigTransmission(const char* data);

        uint8_t baudIndexFromBaudValue(long baudVal);
        long baudIndexToBaudValue(uint8_t index);
};