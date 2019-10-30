#include <Arduino.h>
#include "SPort.h"

#define COMMAND_ID 0x1B
#define PHYSICAL_ID 0x12
#define SENSOR_ID 0x5100
#define SPORT_PIN 3

SPortHub hub(PHYSICAL_ID, SPORT_PIN);
SimpleSPortSensor simpleSensor(SENSOR_ID);

void commandReceived(int prim, int applicationId, int value);

void setup() {
    Serial.begin(115200);

    hub.commandId = COMMAND_ID;
    hub.commandReceived = commandReceived;
    hub.registerSensor(simpleSensor);
    hub.begin();

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    hub.handle();
}

void commandReceived(int prim, int applicationId, int value) {
    Serial.print("PRIM: ");
    Serial.print(prim, HEX);
    Serial.print(", applicationId: ");
    Serial.print(applicationId, HEX);
    Serial.print(", value: ");
    Serial.println(value, HEX);

    analogWrite(LED_BUILTIN, value);
}
