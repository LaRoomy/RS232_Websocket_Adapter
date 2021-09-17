
#include <Arduino.h>

//enum PARITY {NONE, ODD, EVEN};
//struct PARITY {
//    enum p {NONE, ODD, EVEN};
//};

enum class PARITY {NONE = 0, ODD = 1, EVEN = 2};

class SerialTransmissionProcessor
{
    public:
        SerialTransmissionProcessor(){}

        void Send(String data);
        void Send(const char* data);

        void StartReception();

        void Terminate();

    private:
        int transmissionIndex = 0;
        String transmissionData;

};

