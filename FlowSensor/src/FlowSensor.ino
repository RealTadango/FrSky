#include <SPort.h>
#include <EEPROM.h>

#define SPORT_PIN 3
#define SPORT_PHYSICAL_ID 0x12
#define SPORT_COMMAND_ID 0x1B

#define SENSOR_PULSES_IN 2

#define SENSOR_CONF_PULSES  0x5200
#define SENSOR_CONF_TANKSIZE 0x5201

SPortHub hub(SPORT_PHYSICAL_ID, SPORT_PIN);
SimpleSPortSensor usedSensor(0x5200);  
SimpleSPortSensor leftSensor(0x5201);  
SimpleSPortSensor currSensor(0x5210);  

long count_raw = 0;
long count_last = 0;

//This is the pulses per ml * 2
double pulses_cfg = 0;
int tank_size = 0;
long lastTime = 0;

void setup() {
  hub.registerSensor(usedSensor);
  hub.registerSensor(leftSensor);
  hub.registerSensor(currSensor);
  hub.commandId = SPORT_COMMAND_ID;
  hub.commandReceived = commandReceived;
  hub.begin();

  //Sensor input
  pinMode(SENSOR_PULSES_IN, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  attachInterrupt(digitalPinToInterrupt(SENSOR_PULSES_IN), flowTick, CHANGE);

  loadData();
}

void flowTick() {
  count_raw++;
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  hub.handle();
  usedSensor.value = (uint16_t)(count_raw / pulses_cfg);

  if(usedSensor.value >= tank_size) {
    leftSensor.value = 0;
  } else {
    leftSensor.value = tank_size - usedSensor.value;
  }
  
  if (millis() > lastTime + 2000) {
    int diff = millis() - lastTime;
    long timeCount = (count_raw - count_last);

    //calc fuel flow in ml/min
    currSensor.value = (double)timeCount / pulses_cfg * 60 / ((double)diff / 1000);
    
    lastTime = millis();
    count_last = count_raw;
  }
}

void commandReceived(int prim, int applicationId, int value) {
  if(prim == SPORT_HEADER_READ) { //Read
    if(applicationId == SENSOR_CONF_PULSES) {
        hub.sendCommand(SPORT_HEADER_RESPONSE, applicationId, pulses_cfg * 100);
    } else if(applicationId == SENSOR_CONF_TANKSIZE) {
        hub.sendCommand(SPORT_HEADER_RESPONSE, applicationId, tank_size);
    }
  } else if(prim == SPORT_HEADER_WRITE) { //Write
    if(applicationId == SENSOR_CONF_PULSES) {
      pulses_cfg = (double)value / 100;
      saveData();
    } else if(applicationId == SENSOR_CONF_TANKSIZE) {
      tank_size = value;
      saveData();
    }
  }
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

void saveData() {
  writeEEPROMValue(0, pulses_cfg * 100);
  writeEEPROMValue(1, tank_size);
}

void loadData() {
  //default = 5.70 For B.I.O-TECH e.K. FCH-M-POM-LC (G 1/8) WITHOUT the restrictor
  //at 400 ml/s average
  pulses_cfg = readEEPROMValue(0, 570) / (double)100;
  tank_size = readEEPROMValue(1, 1000); //Default tank size to 1000
}
