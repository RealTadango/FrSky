#include <SPort.h>
#include "Ecu_Fadec.h"
#include "Ecu_Jetronic.h"
#include <EEPROM.h>

#define SPORT_PIN 3
#define SPORT_PHYSICAL_ID 0x10
#define SPORT_COMMAND_ID 0x1B

#define SPORT_APPL_ID_TERMINAL_BASE 0x5000
#define TERMINAL_ENABLE 0x10
#define TERMINAL_DISABLE 0x10
#define TERMINAL_KEY 0x20
#define TERMINAL_KEY 0x20
#define TERMINAL_TYPE 0x50

#define TYPE_JETRONIC 0x01
#define TYPE_FADEC 0x02

byte ecuType = 0;
Ecu* ecu = nullptr;

sportData getTerminalData(CustomSPortSensor* sensor);
void commandReceived(int prim, int applicationId, int value);

SPortHub hub(SPORT_PHYSICAL_ID, SPORT_PIN);
CustomSPortSensor terminalSensor(getTerminalData);

//Ecu terminal data
char terminalSentDisplay[32];

void loadEcuType(){
  switch(ecuType) {
    case TYPE_JETRONIC: ecu = new Ecu_Jetronic(); break;
    case TYPE_FADEC: ecu = new Ecu_Fadec(); break;
  }
}

void setup() {
  loadData();

  loadEcuType();

  hub.commandReceived = commandReceived;
  hub.commandId = SPORT_COMMAND_ID;

  if(ecu) {
    ecu->begin();
    ecu->registerSensors(hub);
  }

  terminalSensor.enabled = false;
  hub.registerSensor(terminalSensor);

  hub.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  hub.handle();
  if(ecu) {
    ecu->handle();
  }
}

sportData getTerminalData(CustomSPortSensor* sensor) {
  sportData data;

  if(ecu) {
    for(short pos = 0; pos <= 7; pos++) {
      //Check if this part of the display has changed
      if(ecu->terminalDisplay[pos * 4] != terminalSentDisplay[pos * 4]
        || ecu->terminalDisplay[(pos * 4)+1] != terminalSentDisplay[(pos * 4)+1]
        || ecu->terminalDisplay[(pos * 4)+2] != terminalSentDisplay[(pos * 4)+2]
        || ecu->terminalDisplay[(pos * 4)+3] != terminalSentDisplay[(pos * 4)+3])
      {
        //Update the sent display
        terminalSentDisplay[pos * 4] = ecu->terminalDisplay[pos * 4];
        terminalSentDisplay[(pos * 4)+1] = ecu->terminalDisplay[(pos * 4)+1];
        terminalSentDisplay[(pos * 4)+2] = ecu->terminalDisplay[(pos * 4)+2];
        terminalSentDisplay[(pos * 4)+3] = ecu->terminalDisplay[(pos * 4)+3];

        data.applicationId = SPORT_APPL_ID_TERMINAL_BASE + pos;

        //Prepare the S.Port data
        longHelper lh;

        lh.byteValue[0] = ecu->terminalDisplay[pos * 4];
        lh.byteValue[1] = ecu->terminalDisplay[(pos * 4)+1];
        lh.byteValue[2] = ecu->terminalDisplay[(pos * 4)+2];
        lh.byteValue[3] = ecu->terminalDisplay[(pos * 4)+3];

        data.value = lh.longValue;
        break;
      }
    }
  }

  return data;
}

void commandReceived(int prim, int applicationId, int value) {
  //Skip new command if old command has to be confirmed first
  
  switch(prim) {
    case SPORT_HEADER_READ: {
      if(applicationId == SPORT_APPL_ID_TERMINAL_BASE && value == TERMINAL_TYPE) { //Request type
        hub.sendCommand(SPORT_HEADER_RESPONSE, SPORT_APPL_ID_TERMINAL_BASE + TERMINAL_TYPE, ecuType);
      }
      break;
    }
    case SPORT_HEADER_WRITE: {
      if(applicationId == SPORT_APPL_ID_TERMINAL_BASE) { //ECU Terminal command
        if(value == TERMINAL_ENABLE) {
          //Enable terminal mode
          terminalSensor.enabled = true;
          if(ecu) {
            ecu->enableSensors(false);
          }

          //Reset display buffer
          for(int i = 0; i <= 31; i++) {
            terminalSentDisplay[i] = ' ';
          }
        } else if(value == TERMINAL_DISABLE) {
          //Disable terminal mode
          terminalSensor.enabled = false;
          if(ecu) {
            ecu->enableSensors(true);
          }

        } else if(value >= TERMINAL_KEY && value < TERMINAL_TYPE) {
          //Write keyNumber to ECU key buffer
          if(ecu) {
            ecu->terminalKey = value - TERMINAL_KEY;
          }
        } else if(value >= TERMINAL_TYPE) {
          ecuType = value - TERMINAL_TYPE;
          saveData();
          loadEcuType();
        }
      }
    }
  }
}

void saveData() {
  writeEEPROMValue(0, ecuType);
}

void loadData() {
  ecuType = readEEPROMValue(0, TYPE_JETRONIC);
}

void writeEEPROMValue(int pos, int value) {
  EEPROM.write(pos * 2, lowByte(value));
  EEPROM.write((pos * 2) + 1, highByte(value));
}

int readEEPROMValue(int pos, int value) {
  if(EEPROM.read(pos * 2) == 0xFF && EEPROM.read((pos * 2) + 1) == 0xFF) {
    return value;
  } else {
    return EEPROM.read(pos * 2) + (EEPROM.read((pos * 2) + 1) * 0x100);
  }
}
