#ifndef SPORT_H
#define SPORT_H

#define SPORT_BAUD 57600
#define SPORT_START 0x7E
#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00

#define SENSOR_SIZE 10

#include <Arduino.h>
#include <SoftwareSerial.h>

struct sensorData {
    bool hasData;
    long value;
};

class SPortSensor{
    public:
        SPortSensor(int id, sensorData (*pCallback)(void));
        sensorData getValue();
        int id;
    private:
        sensorData (*getData)(void);
};

struct sensorRegistration {
    int id;
    SPortSensor *sensor; 
};

class SPortHub{
    public:
//        SPortHub(Serial_& serial);
        SPortHub(HardwareSerial& serial);
        SPortHub(int softwarePin);
        void begin();
        void handle();
        void registerSensor(SPortSensor& sensor);
    private:
        void SendData(sensorData data);
//        Serial_* hwStream;
        HardwareSerial* hwStream2;
        SoftwareSerial* swStream;
        int _softwarePin;
        Stream* stream;
        bool valid;
        short index;
        byte buffer[25];
        byte prevValue;
        bool inFrame;
        sensorRegistration sensors[SENSOR_SIZE];
};

#endif