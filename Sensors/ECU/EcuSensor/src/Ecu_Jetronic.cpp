#include "Ecu_Jetronic.h"
#include <Sport.h>

#define TERMINAL_ESC 0x1B

Ecu_Jetronic::Ecu_Jetronic() {
    sensorEGT = new SimpleSPortSensor(0x0400);
    sensorRPM = new SimpleSPortSensor(0x0500);
    sensorBattVoltage = new SimpleSPortSensor(0x0900);
    sensorPumpVoltage = new SimpleSPortSensor(0x0910);
}

void Ecu_Jetronic::begin()
{
  Serial.begin(9600);
}

void Ecu_Jetronic::registerSensors(SPortHub& hub) {
  hub.registerSensor(*sensorEGT);
  hub.registerSensor(*sensorRPM);
  hub.registerSensor(*sensorBattVoltage);
  hub.registerSensor(*sensorPumpVoltage);
}

void Ecu_Jetronic::enableSensors(bool enabled) {
  sensorEGT->enabled = enabled;
  sensorRPM->enabled = enabled;
  sensorBattVoltage->enabled = enabled;
  sensorPumpVoltage->enabled = enabled;
}

void Ecu_Jetronic::handle()
{
  while(Serial.available())
  {
    byte newVal = Serial.read();
    if(newVal == TERMINAL_ESC) { //Skip the esc command
      ecuValid = false;
    } else if(ecuPrev == TERMINAL_ESC && newVal == 0x4B) { // //Request for keycode
      delay(5);
      SendKeyCode();
      ecuValid = false;
    } else if(ecuPrev == 0x43 && newVal == 0x80) { //Start of first LCD line
      ecuIndex = 0;
      ecuValid = true;
    } else if(ecuPrev == 0x43 && newVal == 0xC0) { //Start of second LCD line
      ecuIndex = 16;
      ecuValid = true;
    } else if(ecuValid) { //Store received byte when sequence matches
      terminalDisplay[ecuIndex] = newVal;

      if(ecuIndex == 15) { //End of line one reached, disable storage
        ecuValid = false;
      } else if(ecuIndex == 31) { //End of line two reached, disable storage and handle the full frame
        ecuValid = false;
        HandleEvojetFrame();
      } else { //Advance to the next position
        ecuIndex++;
      }
    }

    ecuPrev = newVal;
  }
}

void Ecu_Jetronic::HandleEvojetFrame() {
  //Led on indicates data from ECU
  digitalWrite(LED_BUILTIN, HIGH);

  byte bcdData[4];

//Read other sensor data later
/*
  if(display[15] == (int)'%') 
  { 
    status = 0x23; 

    bcdData[0] = display[14];
    bcdData[1] = display[13];
    bcdData[2] = display[12];

    throttle = FromBCD(bcdData, 3);
  }
  else if(display[12] == (int)'-') { status = 0x20; }
  //else if(display[12] == (int)'f') { status = 0x21; }
  else if(display[12] == (int)'a') { status = 0x22; }
  else if(display[12] == (int)'r') { status = 0x23; }
  else if(display[12] == (int)'c' && display[13] == (int)'a') { status = 0x24; }
  else if(display[12] == (int)'c' && display[13] == (int)'o') { status = 0x25; }
  else if(display[12] == (int)'f') { status = 0x26; }
  //else if(display[12] == (int)'f') { status = 0x27; }
  else if(display[12] == (int)'h') { status = 0x28; }
  else if(display[12] == (int)'i') { status = 0x29; }
  else if(display[12] == (int)'l') { status = 0x2A; }
  else if(display[12] == (int)'r') { status = 0x2B; }
  else if(display[12] == (int)'s' && display[13] == (int)'p') { status = 0x2C; }
  else if(display[12] == (int)'s' && display[13] == (int)'t') { status = 0x2D; }
*/
  if(sensorEGT->enabled) {
    bcdData[0] = terminalDisplay[5];
    bcdData[1] = terminalDisplay[4];
    bcdData[2] = terminalDisplay[3];

    sensorEGT->value = FromBCD(bcdData, 3);
  }

  if(sensorRPM->enabled) {
    bcdData[0] = terminalDisplay[20];
    bcdData[1] = terminalDisplay[18];
    bcdData[2] = terminalDisplay[17];
    bcdData[3] = terminalDisplay[16];

    sensorRPM->value = (uint32_t)FromBCD(bcdData, 4) * 100;
  }

  if(sensorBattVoltage->enabled && terminalDisplay[25] == (int)'U')
  {
    bcdData[0] = terminalDisplay[30];
    bcdData[1] = terminalDisplay[28];
    bcdData[2] = terminalDisplay[27];

    sensorBattVoltage->value = FromBCD(bcdData, 3);
  }

  if(sensorPumpVoltage->enabled && terminalDisplay[25] == (int)'F')
  {
    bcdData[0] = terminalDisplay[30];
    bcdData[1] = terminalDisplay[29];
    bcdData[2] = terminalDisplay[27];

    sensorPumpVoltage->value = FromBCD(bcdData, 3) / (double)10;
  }
}

//Get a value from the BCD chars
int Ecu_Jetronic::FromBCD(byte bcdData[], int length) {
  int result = 0;

  for(int i = 0; i < length; i++) {
    if(bcdData[i] >= (int)'0' && bcdData[i] <= (int)'9') {
      int val = (bcdData[i] - (int)'0');

      if(i == 1) {
        val = val * 10;
      } else if(i == 2) {
        val = val * 100;
      } else if(i == 3) {
        val = val * 1000;
      }

      result += val;
    }
  }

  return result;
}

void Ecu_Jetronic::SendKeyCode() {
  if(terminalKey == 0 && enterKeyRepeat == 0) {
    return;
  }
  
  if(terminalKey == 1 || terminalKey == 2 || enterKeyRepeat > 0) {
    if(terminalKey == 1) {
      enterKeyRepeat = 15;  //Send 15 times the enter key
    } else if(enterKeyRepeat > 0) {
      enterKeyRepeat--;
    }

    //Enter
    Serial.write(0x7F);
  } else if(terminalKey == 3) {
    //Up
    Serial.write(0xBF);
  } else if(terminalKey == 4) {
    //Down
    Serial.write(0xDF);
  }

  terminalKey = 0;
}