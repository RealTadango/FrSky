#include <SPort.h>                  //Include the SPort library

SPortHub hub(0x12, 3);              //Hardware ID 0x12, Software serial pin 3
SimpleSPortSensor sensor(0x5900);   //Sensor with ID 0x5900

void setup() {
  hub.registerSensor(sensor);       //Add sensor to the hub
  hub.begin();                      //Start listening
}

void loop() {
  sensor.value = 1234;              //Set the sensor value
  hub.handle();                     //Handle new data
}
