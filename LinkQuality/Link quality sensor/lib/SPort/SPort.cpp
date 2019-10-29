#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SPort.h"


// SPortHub::SPortHub(Serial_& serial) {
//     hwStream = &serial;
// }

SPortHub::SPortHub(HardwareSerial& serial) {
    hwStream2 = &serial;
    stream = &serial;
}

SPortHub::SPortHub(int softwarePin) {
    swStream = &SoftwareSerial(softwarePin, softwarePin, true);
    _softwarePin = softwarePin;
    stream = swStream;
}

void SPortHub::begin() {
    // if(hwStream) {
    //     hwStream->begin(SPORT_BAUD, SERIAL_8E2);
    // } else 
    if(hwStream2) {
        hwStream2->begin(SPORT_BAUD, SERIAL_8E2);
    } else if(swStream) {
        swStream->begin(SPORT_BAUD);
    }
}

void SPortHub::handle() {
    while(stream->available()) {
        byte newByte = stream->read();

        if(newByte == SPORT_START) {
            valid = true;
            index = 0;
        } else if(index == 1) {
            int physicalID = newByte & 0x1F;

            for(int i = 0; i < SENSOR_SIZE; i++)
            {
                if(sensors[i].id == physicalID){
                    sensorData data = sensors[i].sensor->getValue();
                    SendData(data);
                    break;
                }
            }
        }

        if(valid)
        {
            buffer[index] = newByte;
            index++;
        }
    }
}

void SPortHub::registerSensor(SPortSensor& sensor) {
    for(int i = 0; i < SENSOR_SIZE; i++)
    {
        if(sensors[i].id == 0){
            sensors[i].id = sensor.id;
            sensors[i].sensor = &sensor;
            break;
        }
    }
}

SPortSensor::SPortSensor(int sensorId, sensorData (*getValueFunc)(void)) {
    id = sensorId;
    getData = getValueFunc;
}

sensorData SPortSensor::getValue() {
    return getData();
}

void SPortHub::SendData(sensorData data) {
    if(swStream) {
        pinMode(_softwarePin, OUTPUT);
    }

    //TODO

    if(swStream) {
        pinMode(_softwarePin, INPUT);
    }
}