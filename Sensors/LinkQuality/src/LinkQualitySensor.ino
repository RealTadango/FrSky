#include <SPort.h>

#define SBUS_BAUD 100000
#define SBUS_THRESHOLD 5
#define SBUS_BUFFER 25
#define FRAMELOSS_BUFFER 100

#define PHYSICAL_ID 0x12
#define SENSOR_ID 0x5100
#define SPORT_PIN 3

SPortHub hub(PHYSICAL_ID, SPORT_PIN);
SimpleSPortSensor qualitySensor(SENSOR_ID);

byte sbusBuffer[SBUS_BUFFER];
long sbusIndex = 0;
byte sbusPreviousValue = 0;
bool sbusInFrame = false;

bool lastFrameLossesBuffer[FRAMELOSS_BUFFER];
int lastFrameLossesIndex = 0;

void setup() {
    Serial.begin(SBUS_BAUD, SERIAL_8E2);

    hub.registerSensor(qualitySensor);
    hub.begin();
}

void loop() {
    hub.handle();

    while (Serial.available()) {
        //Read MSB value from S.Bus stream
        byte value = Serial.read();

        //Only start a new frame if everything lines up correctly
        if(!sbusInFrame && value == 0x0F && sbusPreviousValue == 0x00) {
            sbusIndex = 0;
            sbusInFrame = true;
        }

        //When in frame, store the value in the buffer
        if(sbusInFrame) {
            sbusBuffer[sbusIndex] = value;
            sbusIndex++;
            
            //If all 25 bytes are received in the frame, handle it
            if(sbusIndex == 25) {
                sbusInFrame = false; //Stop capturing
                handleSBusFrame();
            }
        }

        sbusPreviousValue = value;
    }
}

void handleSBusFrame() {   
  bool framelost = sbusBuffer[23] & 0x04;

  lastFrameLossesBuffer[lastFrameLossesIndex] = framelost;
  lastFrameLossesIndex++;
  if(lastFrameLossesIndex >= FRAMELOSS_BUFFER) {
    lastFrameLossesIndex = 0;
  }
  
  int lastLost = 0;
  
  for(int i = 0; i < FRAMELOSS_BUFFER; i++) {
      if(lastFrameLossesBuffer[i]) {
        lastLost++;
      }
  }

  qualitySensor.value = 100 - (((double)lastLost / (double)FRAMELOSS_BUFFER) * (double)100);
}