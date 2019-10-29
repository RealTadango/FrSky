#ifndef SPORT_H
#define SPORT_H

#define SPORT_BAUD 57600
#define SPORT_START 0x7E
#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00

#define MAX_SENSOR_COUNT 10

#include <Arduino.h>
#include <SoftwareSerial.h>

typedef union {
    char byteValue[4];
    long longValue;
} longHelper;

struct sensorData {
    long value;
    int sensorId;
};

class SPortSensor{
    public:
        SPortSensor(int id, sensorData (*pCallback)(void));
        sensorData getValue();
        int id;
    private:
        sensorData (*getData)(void);
};

class SPortHub{
    public:
#ifdef Serial_
        SPortHub(Serial_& serial);
#else
        SPortHub(HardwareSerial& serial);
#endif
        SPortHub(int softwarePin);
        void begin();
        void handle();
        void registerSensor(SPortSensor& sensor);
    private:
        void SendData(sensorData data);
        void SendByte(byte b);
        byte GetChecksum(byte data[], int start, int len);

#ifdef Serial_
        Serial_* _hwStream;
#else
        HardwareSerial* _hwStream;
#endif
        SoftwareSerial* _swStream;
        Stream* _stream;
        int _softwarePin;
        bool _valid;
        short _index;
        byte _buffer[25];
        // byte prevValue;
        // bool inFrame;
        SPortSensor *_sensors[MAX_SENSOR_COUNT];
};

#endif