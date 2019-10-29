#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SPort.h"

SPortHub hub(3);
// SPortHub hub(Serial);

sensorData sensorCallback();

SPortSensor sensor(12, sensorCallback);

void setup() {
    hub.registerSensor(sensor);
    hub.begin();
}

void loop() {
    hub.handle();
}

sensorData sensorCallback() {
    sensorData answer = { true, 123 };

    return answer;
}