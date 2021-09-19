#include "serial.h"
#include <limits.h>

void SerialTransmissionHandler::config(unsigned long baudrate,DATABITS db, PARITY p, STOPPBITS sb){

    this->baud = baudrate;
    
    if(p == PARITY::NONE){
        if(sb == STOPPBITS::ONE){
            switch (db)
            {
            case DATABITS::FIVE:
                this->conf = SERIAL_5N1;
                break;
            case DATABITS::SIX:
                this->conf = SERIAL_6N1;
                break;
            case DATABITS::SEVEN:
                this->conf = SERIAL_7N1;
                break;
            case DATABITS::EIGHT:
                this->conf = SERIAL_8N1;
                break;            
            default:
                break;
            }
        }
        else if(sb == STOPPBITS::TWO){
            switch (db)
            {
            case DATABITS::FIVE:
                this->conf = SERIAL_5N2;
                break;
            case DATABITS::SIX:
                this->conf = SERIAL_6N2;
                break;
            case DATABITS::SEVEN:
                this->conf = SERIAL_7N2;
                break;
            case DATABITS::EIGHT:
                this->conf = SERIAL_8N2;
                break;            
            default:
                break;
            }
        }
    }
    else if(p == PARITY::EVEN){
        if(sb == STOPPBITS::ONE){
            switch (db)
            {
            case DATABITS::FIVE:
                this->conf = SERIAL_5E1;
                break;
            case DATABITS::SIX:
                this->conf = SERIAL_6E1;
                break;
            case DATABITS::SEVEN:
                this->conf = SERIAL_7E1;
                break;
            case DATABITS::EIGHT:
                this->conf = SERIAL_8E1;
                break;            
            default:
                break;
            }
        }
        else if(sb == STOPPBITS::TWO){
            switch (db)
            {
            case DATABITS::FIVE:
                this->conf = SERIAL_5E2;
                break;
            case DATABITS::SIX:
                this->conf = SERIAL_6E2;
                break;
            case DATABITS::SEVEN:
                this->conf = SERIAL_7E2;
                break;
            case DATABITS::EIGHT:
                this->conf = SERIAL_8E2;
                break;            
            default:
                break;
            }
        }
    }
    else if(p == PARITY::ODD){
        if(sb == STOPPBITS::ONE){
            switch (db)
            {
            case DATABITS::FIVE:
                this->conf = SERIAL_5O1;
                break;
            case DATABITS::SIX:
                this->conf = SERIAL_6O1;
                break;
            case DATABITS::SEVEN:
                this->conf = SERIAL_7O1;
                break;
            case DATABITS::EIGHT:
                this->conf = SERIAL_8O1;
                break;            
            default:
                break;
            }
        }
        else if(sb == STOPPBITS::TWO){
            switch (db)
            {
            case DATABITS::FIVE:
                this->conf = SERIAL_5O2;
                break;
            case DATABITS::SIX:
                this->conf = SERIAL_6O2;
                break;
            case DATABITS::SEVEN:
                this->conf = SERIAL_7O2;
                break;
            case DATABITS::EIGHT:
                this->conf = SERIAL_8O2;
                break;            
            default:
                break;
            }
        }
    }   
}

void SerialTransmissionHandler::sendData(){
    this->transmissionIndex = 0;
    this->currentTransmission = TRANSMISSION_TYPE::SEND;

    Serial.begin(this->baud, this->conf);
}

void SerialTransmissionHandler::sendData(String data){
    this->transmissionData = data;
    this->transmissionIndex = 0;
    this->currentTransmission = TRANSMISSION_TYPE::SEND;

    Serial.begin(this->baud, this->conf);
}

void SerialTransmissionHandler::setTransmissionData(String data, bool eraseHeader, unsigned int headerSize){
    if(!eraseHeader){
        this->transmissionData = data;
    } else {
        this->transmissionData.clear();

        for(unsigned int i = 5; i < data.length(); i++){
            this->transmissionData += data.charAt(i);
        }
    }
}

void SerialTransmissionHandler::addTransmissionData(String data, bool eraseHeader, unsigned int headerSize){
    if(!eraseHeader){
        this->transmissionData += data;
    } else {
        for(unsigned int i = 5; i < data.length(); i++){
            this->transmissionData += data.charAt(i);
        }
    }
}


void SerialTransmissionHandler::processSerialTransmission(){

    if(this->currentTransmission == TRANSMISSION_TYPE::SEND){
        if(this->transmissionIndex < this->transmissionData.length()){
            Serial.write(
                transmissionData.charAt(transmissionIndex)
                );
            transmissionIndex++;
        }
        else
        {
            // send complete
            Serial.flush();
            Serial.end();

            if(this->eventHandler != nullptr){
                this->eventHandler->onSendComplete(transmissionIndex);
            }

            this->currentTransmission = TRANSMISSION_TYPE::NONE;
            this->transmissionIndex = 0;
            this->transmissionData.clear();            
        }
    }
    else if(this->currentTransmission == TRANSMISSION_TYPE::RECEIVE){
        if(Serial.available() > 0){
            char c = (char)Serial.read();
            this->transmissionData += c;
            transmissionIndex++;

            // detect finalization!

        }
    }    
}


void SerialTransmissionHandler::reset(){

    if(this->currentTransmission != TRANSMISSION_TYPE::NONE){
        this->terminate();
    }
    this->transmissionIndex = 0;
    this->transmissionData.clear();
}

void SerialTransmissionHandler::terminate(){
    if(this->currentTransmission != TRANSMISSION_TYPE::NONE){
        Serial.end();
    }
    this->transmissionIndex = 0;
    this->transmissionData.clear();
    this->currentTransmission = TRANSMISSION_TYPE::NONE;
}