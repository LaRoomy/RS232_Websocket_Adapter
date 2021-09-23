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
#include <ESP8266mDNS.h>

// this enables some functions which are only intended for testing purposes
//#define     EVALUATION_MODE


#include "serial.h"

#include <EEPROM.h>
#define     EE_BAUD_ADDR                0x00
#define     EE_DATAB_ADDR               0x01
#define     EE_PARITY_ADDR              0x02
#define     EE_STOPPB_ADDR              0x03
#define     EE_AD_EOT_ADDR              0x04
// free (for future parameter)
#define     EE_WLAN_SSID_LENGTH_ADDR    0x08
#define     EE_WLAN_PASSWRD_LENGTH_ADDR 0x09
#define     EE_WLAN_SSID_START_ADDR     0x0A
#define     EE_WLAN_SSID_MAX_ADDR       0x10A
#define     EE_WLAN_PASSWRD_START_ADDR  0x10B
#define     EE_WLAN_PASSWRD_MAX_ADDR    0x20B

#define     EE_PROM_SIZE                 524

#include "stringTable.h"

#define     INVALID_BAUD_INDEX          99

enum class DEVICEMODE {SOCKETMODE, CONFIGMODE};
enum class INPUTMODE {NONE, SSID_MODE, PASSWORD_MODE};

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
        void onReceptionComplete(const String& buffer, size_t size);
        void onReceptionStarted();
        void onError(TRANSMISSION_ERROR error);

        //void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

        void notifyClients(String msg){
            this->ws.textAll(msg);
        }

        // this is temp: this is only a placeholder for the dynamic values implemented later

        // network credentials work
        //const char* ssid = "CT Workstation AP";
        //const char* password = "30321065";

        // network credentials home
        //const char* ssid = "Delker Zimmi Net";
        //const char* password = "60458561901103846208";


    private:
        AsyncWebServer server = AsyncWebServer(80);
        AsyncWebSocket ws = AsyncWebSocket("/ws");
        
        SerialTransmissionHandler sHandler;

        String network_ssid;
        String network_password;

        bool isConnected = false;
        bool autoDetectEOT = true;

        DEVICEMODE mode = DEVICEMODE::SOCKETMODE;
        INPUTMODE inputMode = INPUTMODE::NONE;

        // Default Serial configuration
        unsigned int scBaudRate = 115200;
        PARITY scParity = PARITY::NONE;
        DATABITS scDatabits = DATABITS::EIGHT;
        STOPPBITS scStoppbits = STOPPBITS::ONE;

        void initWebSocket() {
            this->ws.onEvent(RootComponent::onEvent);
            this->server.addHandler(&ws);
        }

        void reConnect();
        void enterConfigMode();
        void leaveConfigMode();

        void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

        static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len);

        void loadPersistentData();

        void notifyUser(String msg);

        void onBaudrateConfigTransmission(const char* data);
        void onDatabitConfigTransmission(const char* data);
        void onParityConfigTransmission(const char* data);
        void onStoppbitConfigTransmission(const char* data);
        void onAutoDetectConfigTransmission(const char* data);
        void onReceiveCommand();
        void onStopReception();
        void onConfigurationRequest();
        void onResetCommand();
        void onHandleTerminalCommunication(const String& data);

        uint8_t baudIndexFromBaudValue(long baudVal);
        long baudIndexToBaudValue(uint8_t index);
};