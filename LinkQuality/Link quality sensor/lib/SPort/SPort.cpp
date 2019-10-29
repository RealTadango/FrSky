#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SPort.h"

SPortHubWHS::SPortHubWHS(Serial_ &serial) {
    _serial = serial;
}

SPortHubSS::SPortHubSS(SoftwareSerial &serial) {
    _serial = serial;
}

void SPortHubWHS::begin() {
    _serial.begin(57600, SERIAL_8E2);
}

void SPortHubSS::begin() {
    _serial.begin(57600);
}