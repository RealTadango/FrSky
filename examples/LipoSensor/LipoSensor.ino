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

int cell = 0;

sportData getSensorData(CustomSPortSensor* sensor) {
  sportData data; 

  data.applicationId = 0x0300;            //Set the sensor id for the current data poll. Set to 0 to discard the data, skip to the next sensor

  data.value = cell;                      //Bit 0-3: Cell id, 0 = 1&2, 2 = 3&4, 4 = 5&6
  data.value |= 6 << 4;                   //Bit 4-7: Total cell count (6 in this demo)

  if (cell == 0)
  {
    data.value |= (uint32_t)(3.81 * 500) << 8;   //Bit 8-19: Cell 1 voltage (0.002 resolution)
    data.value |= (uint32_t)(3.82 * 500) << 20;  //Bit 20-31: Cell 2 voltage (0.002 resolution)
  } 
  else if (cell == 2)
  {
    data.value |= (uint32_t)(3.83 * 500) << 8;   //Bit 8-19: Cell 3 voltage (0.002 resolution)
    data.value |= (uint32_t)(3.84 * 500) << 20;  //Bit 20-31: Cell 4 voltage (0.002 resolution)
  }
  else if (cell == 4)
  {
    data.value |= (uint32_t)(3.85 * 500) << 8;   //Bit 8-19: Cell 5 voltage (0.002 resolution)
    data.value |= (uint32_t)(3.86 * 500) << 20;  //Bit 20-31: Cell 6 voltage (0.002 resolution)
  }

  cell += 2;

  if (cell > 4)
  {
    cell = 0;
  }
  
  return data;
}
