#include <Arduino.h>
#include "SPort.h"

#define PHYSICAL_ID 0x12
#define SPORT_PIN 3

SPortHub hub(SPORT_PIN); //Software serial, single pin
// SPortHub hub(Serial); //Hardware serial (needs hardware inverter)

sensorData sensorCallback();

SPortSensor sensor(PHYSICAL_ID, sensorCallback);

void setup() {
    hub.registerSensor(sensor);
    hub.begin();
}

void loop() {
    hub.handle();
}

sensorData sensorCallback() {
    sensorData answer;
    answer.value = 123;
    answer.sensorId = 0x5299;
    return answer;
}