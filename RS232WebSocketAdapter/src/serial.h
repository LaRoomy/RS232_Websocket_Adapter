
#include <Arduino.h>

//enum PARITY {NONE, ODD, EVEN};
//struct PARITY {
//    enum p {NONE, ODD, EVEN};
//};

enum class PARITY {NONE = 0, ODD = 1, EVEN = 2};
enum class STOPPBITS {ONE = 1, TWO = 2};

class ISerialTransmissionEvents {
    virtual void onReceptionComplete(const char* buffer) = 0;
};

class SerialTransmissionHandler
{
    public:
        SerialTransmissionHandler(){
            this->transmissionIndex = 0;
        }
        ~SerialTransmissionHandler(){}

        void config(unsigned long baudrate ,int parity, int stoppBits);

        void sendData(String data);
        void sendData(const char* data){
            this->sendData(String(data));
        }

        // put this to loop() to handle the serial transmisson async
        void processSerialTransmission();

        void startReception();


        void reset();
        void terminate();

        void subscribeHandler(ISerialTransmissionEvents* handler){
            this->eventHandler = handler;
        }

    private:
        int transmissionIndex = 0;
        String transmissionData;
        ISerialTransmissionEvents* eventHandler = nullptr;

};

