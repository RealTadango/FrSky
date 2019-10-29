#ifndef SPORT_H
#define SPORT_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class SPortHubWHS{
    public:
        SPortHubWHS(Serial_ &serial);
        void begin();

    private:
        Serial_ _serial;
};

class SPortHubSS{
    public:
        SPortHubSS(SoftwareSerial &serial);
        void begin();

    private:
        SoftwareSerial _serial;
};



#endif