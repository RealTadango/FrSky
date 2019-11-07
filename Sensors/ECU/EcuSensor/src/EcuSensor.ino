#include <SPort.h>

//#define ECU_JETRONIC
//#define ECU_FADEC

#define SPORT_PIN 3
#define SPORT_PHYSICAL_ID 0x10
#define SPORT_COMMAND_ID 0x1B

#define SPORT_APPL_ID_TERMINAL 0x5000  

#define CMD_ENABLE_TERMINAL 0x10
#define CMD_DISABLE_TERMINAL 0x11

Ecu_Fadec ecu();

sportData getTerminalData(CustomSPortSensor* sensor);
void commandReceived(int prim, int applicationId, int value);

SPortHub hub(SPORT_PHYSICAL_ID, SPORT_PIN);
CustomSPortSensor terminalSensor(getTerminalData);

//Ecu terminal data
byte terminalSentDisplay[32];
byte terminalDisplay[32] = "ECU Sensor V1.1 Herman Kruisman";

void setup() {
  hub.commandReceived = commandReceived;
  hub.commandId = SPORT_COMMAND_ID;

  ecu.registerSensors(hub);

  terminalSensor.enabled = false;
  hub.registerSensor(terminalSensor);

  hub.begin();
  ecu.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  hub.handle();
  ecu.handle();
}

sportData getTerminalData(CustomSPortSensor* sensor) {
  sportData data;

  for(short pos = 0; pos <= 7; pos++) {
    //Check if this part of the display has changed
    if(terminalDisplay[pos * 4] != terminalSentDisplay[pos * 4]
      || terminalDisplay[(pos * 4)+1] != terminalSentDisplay[(pos * 4)+1]
      || terminalDisplay[(pos * 4)+2] != terminalSentDisplay[(pos * 4)+2]
      || terminalDisplay[(pos * 4)+3] != terminalSentDisplay[(pos * 4)+3])
    {
      //Update the sent display
      terminalSentDisplay[pos * 4] = terminalDisplay[pos * 4];
      terminalSentDisplay[(pos * 4)+1] = terminalDisplay[(pos * 4)+1];
      terminalSentDisplay[(pos * 4)+2] = terminalDisplay[(pos * 4)+2];
      terminalSentDisplay[(pos * 4)+3] = terminalDisplay[(pos * 4)+3];

      data.applicationId = SPORT_APPL_ID_TERMINAL + pos;

      //Prepare the S.Port data
      longHelper lh;

      lh.byteValue[0] = terminalDisplay[pos * 4];
      lh.byteValue[1] = terminalDisplay[(pos * 4)+1];
      lh.byteValue[2] = terminalDisplay[(pos * 4)+2];
      lh.byteValue[3] = terminalDisplay[(pos * 4)+3];

      data.value = lh.longValue;
      break;
    }
  }

  return data;
}

void commandReceived(int prim, int applicationId, int value) {
  //Skip new command if old command has to be confirmed first
  
  if(applicationId == SPORT_APPL_ID_TERMINAL && prim == SPORT_HEADER_WRITE) { //ECU Terminal command
    if(value == CMD_ENABLE_TERMINAL) {
      //Enable terminal mode
      terminalSensor.enabled = true;
      ecu.enableSensors(false);

      //Reset display buffer
      for(int i = 0; i <= 31; i++) {
        terminalSentDisplay[i] = ' ';
      }
    } else if(value == CMD_DISABLE_TERMINAL) {
      //Disable terminal mode
      terminalSensor.enabled = false;
      ecu.enableSensors(true);

    } else if(value >= 0x20) {
      //Write keyNumber to ECU key buffer
      ecu.terminalKey = value - 0x20;
    } 
  }
}