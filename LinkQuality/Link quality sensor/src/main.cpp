#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SPort.h"

SoftwareSerial ss(3,3, true);
SPortHubSS hub(ss);

void setup() {
    hub.begin();
}

void loop() {
}