#include <SPort.h>                                    //Include the SPort library

SPortHub hub(0x12, 3);                                //Hardware ID 0x12, Software serial pin 3
SimpleSPortSensor sensor(0x5900);                     //Sensor with ID 0x5900

void setup() {
  hub.commandId = 0x1B;                               //Listen to data send to thist physical ID
  hub.commandReceived = handleCommand;                //Function to handle received commands
  hub.registerSensor(sensor);                         //Add sensor to the hub
  hub.begin();                                        //Start listening
}

void loop() {
  sensor.value = 1234;                                //Set the sensor value
  hub.handle();                                       //Handle new data
}

void handleCommand(int prim, int applicationId, int value) {
  hub.sendCommand(0x32, applicationId, value + 1);    //Send a command back to lua, with 0x32 as reply and the value + 1
}