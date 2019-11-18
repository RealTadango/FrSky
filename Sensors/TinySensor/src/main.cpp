#include <Arduino.h>
#include <SPort.h>

SPortHub hub(0x12, 0);
SimpleSPortSensor sensor(0x400);

void send(SPortSensor *sensor) {
  digitalWrite(1, HIGH);
}

void setup() {
  OSCCAL = 105;

  sensor.valueSend = send;

  hub.registerSensor(sensor);
  hub.begin();

  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
}

void loop() {
  sensor.value = 1234;
  hub.handle();
}