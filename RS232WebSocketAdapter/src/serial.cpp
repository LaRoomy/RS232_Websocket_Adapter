#include "serial.h"

void SerialTransmissionHandler::config(unsigned long baudrate ,int parity, int stoppBits){

    // get the config type 7E1 for example
    // save it for the begin of the session
    
}

void SerialTransmissionHandler::sendData(String data){
    this->transmissionData = data;
}


void SerialTransmissionHandler::processSerialTransmission(){
    
}


void SerialTransmissionHandler::reset(){
    this->transmissionIndex = 0;
    this->transmissionData.clear();
}