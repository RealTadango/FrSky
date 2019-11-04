#include <SPort.h>                        //Include the SPort library

SPortHub hub(0x12, 3);                    //Hardware ID 0x12, Software serial pin 3
CustomSPortSensor sensor(getSensorData);  //Sensor with a callback function to get the data

void setup() {
  hub.registerSensor(sensor);             //Add sensor to the hub
  hub.begin();                            //Start listening
}

void loop() {
  hub.handle();                           //Handle new data
}

sportData getSensorData(CustomSPortSensor* sensor) {
  sportData data; 

  data.applicationId = 0x5900;            //Set the sensor id for the current data poll. Set to 0 to discard the data, skip to the next sensor
  data.value = 1234;                      //Set the sensor value 

  return data;
}