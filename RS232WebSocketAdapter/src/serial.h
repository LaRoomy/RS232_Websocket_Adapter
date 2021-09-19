#include <Arduino.h>

enum class DATABITS {FIVE = 5, SIX = 6, SEVEN = 7, EIGHT = 8};
enum class PARITY {NONE = 0, ODD = 1, EVEN = 2};
enum class STOPPBITS {ONE = 1, TWO = 2};
enum class TRANSMISSION_TYPE {NONE = 0, SEND = 1, RECEIVE = 2};

class ISerialTransmissionEvents {
    public:
        virtual void onSendComplete(unsigned int bytesTransferred) = 0;
        virtual void onReceptionComplete(const char* buffer, size_t size) = 0;
};

class SerialTransmissionHandler
{
    public:
        SerialTransmissionHandler(){
            this->transmissionIndex = 0;
        }
        ~SerialTransmissionHandler(){}

        void config(unsigned long baudrate ,DATABITS db ,PARITY p, STOPPBITS sb);

        void sendData();
        void sendData(String data);
        void sendData(const char* data){
            this->sendData(String(data));
        }

        void setTransmissionData(String data){
            this->transmissionData = data;
        }
        void setTransmissionData(const char* data){
            this->setTransmissionData(String(data));
        }
        void setTransmissionData(String data, bool eraseHeader, unsigned int headerSize);
        
        void addTransmissionData(String data, bool eraseHeader, unsigned int headerSize);

        // put this to loop() to handle the serial transmisson async
        void processSerialTransmission();

        void startReception();


        void reset();
        void terminate();

        void subscribeHandler(ISerialTransmissionEvents* handler){
            this->eventHandler = handler;
        }

        void operator+= (ISerialTransmissionEvents* handler){
            this->eventHandler = handler;
        }

    private:
        unsigned int transmissionIndex = 0;
        SerialConfig conf = SERIAL_8N1;// default
        unsigned long baud = 9600;// default
        TRANSMISSION_TYPE currentTransmission = TRANSMISSION_TYPE::NONE;
        String transmissionData;
        ISerialTransmissionEvents* eventHandler = nullptr;

};

