#include <Arduino.h>
#include "SPort.h"

#define PHYSICAL_ID 0x12
#define SENSOR_ID1 0x5100
#define SENSOR_ID2 0x5101
#define SENSOR_ID3 0x5102
#define SPORT_PIN 3

void increase(); //Declare function
sensorData getData(CustomSPortSensor* sensor); //Declare function

SPortHub hub(PHYSICAL_ID, SPORT_PIN); //Software serial, single pin
SimpleSPortSensor simpleSensor(SENSOR_ID1);
SimpleSPortSensor simpleSensor2(SENSOR_ID2);
CustomSPortSensor complexSensor(getData);

void setup() {
    hub.registerSensor(simpleSensor);

    simpleSensor2.valueSend = increase;
    hub.registerSensor(simpleSensor2);

    hub.registerSensor(complexSensor);

    hub.begin();
}

void loop() {
    simpleSensor.value = 42;
    hub.handle();
}

void increase() { //Implement function
    simpleSensor2.value++;
    if(simpleSensor2.value >= 100) {
        simpleSensor2.value = 0;
    }
}

sensorData getData(CustomSPortSensor* sensor) {
    sensorData result;
    result.sensorId = SENSOR_ID3;
    result.value = 1234;
    return result;
}