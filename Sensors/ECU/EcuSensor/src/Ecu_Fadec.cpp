#include "Ecu_Fadec.h"
#include <Sport.h>

Ecu_Fadec::Ecu_Fadec() {
    sensorEGT = new SimpleSPortSensor(0x0400);
    sensorRPM = new SimpleSPortSensor(0x0500);
    sensorCurrent = new SimpleSPortSensor(0x0200);
    sensorBattVoltage = new SimpleSPortSensor(0x0900);
    sensorPumpVoltage = new SimpleSPortSensor(0x0910);
}

void Ecu_Fadec::begin() {
    Serial.begin(4800);
}

void Ecu_Fadec::registerSensors(SPortHub& hub) {
    hub.registerSensor(*sensorEGT);
    hub.registerSensor(*sensorRPM);
    hub.registerSensor(*sensorCurrent);
    hub.registerSensor(*sensorBattVoltage);
    hub.registerSensor(*sensorPumpVoltage);
}

void Ecu_Fadec::handle() {
    while(Serial.available()) {
        byte newVal = Serial.read();
        if(newVal == 253 && ecuPrev == 252) { //New frame!
            ecuIndex = 1;
            ecuValid = true;
        }

        if(ecuValid) {
            ecuBuffer[ecuIndex] = newVal;

            if(ecuIndex == 49) {
                ecuValid = false;
                SendKeyCode();
                HandleXicoyFrame();
            }

            ecuIndex++;
        }

        ecuPrev = newVal;
    }
};

void Ecu_Fadec::enableSensors(bool enabled) {
    sensorEGT->enabled = enabled;
    sensorRPM->enabled = enabled;
    sensorCurrent->enabled = enabled;
    sensorBattVoltage->enabled = enabled;
    sensorPumpVoltage->enabled = enabled;
};

void Ecu_Fadec::SendKeyCode() {
    if(terminalKey == 0) {
        return;
    }

    delay(2);

    //Setup key command
    byte data[26];
    data[0] = 0xDE;
    data[1] = 0xDF;
    data[2] = 0x70;

    //Reset all else to 0
    for(int i = 5; i < 25; i++) {
        data[i] = 0;
    }

    data[25] = 0xFF;

    if(terminalKey == 1) { //data down
        data[3] = 0x42;
        data[4] = 0xB2;
    } else if(terminalKey == 2) { //data up
        data[3] = 0x41;
        data[4] = 0xB1;
    } else if(terminalKey == 3) { //menu up
        data[3] = 0x43;
        data[4] = 0xB3;
    } else if(terminalKey == 4) { //menu down
        data[3] = 0x44;
        data[4] = 0xB4;
    }

    Serial.write(data, 26);

    terminalKey = 0;
}

void Ecu_Fadec::HandleXicoyFrame() {
    //Led on indicates data from ECU
    digitalWrite(LED_BUILTIN, HIGH);

    int keyByte = 35;

    //Correct frame bytes
    int key = ecuBuffer[keyByte];

    for(int i = 2; i < 50; i++) {
        byte val = 255 - ecuBuffer[i] + key;

        if(val > 255) {
        val -= 255;
        }

        ecuBuffer[i] = val;
    }

    for(int i = 0; i < 32; i++) { //Create display string
        terminalDisplay[i] = ecuBuffer[i + 2];
    }

    /*
    if(display[0] == (int)'H' && display[1] == (int)'i') { status = 0x50; }
    else if(display[0] == (int)'T' && display[1] == (int)'r') { status = 0x51; }
    else if(display[0] == (int)'S' && display[2] == (int)'i') { status = 0x52; }
    else if(display[0] == (int)'R' && display[1] == (int)'e') { status = 0x53; }
    else if(display[0] == (int)'I') { status = 0x54; }
    else if(display[0] == (int)'F' && display[1] == (int)'u') { status = 0x55; }
    else if(display[0] == (int)'G' && display[5] == (int)'T') { status = 0x56; }
    else if(display[0] == (int)'R' && display[1] == (int)'u') { status = 0x57; }
    else if(display[0] == (int)'S' && display[2] == (int)'o') { status = 0x58; }
    else if(display[0] == (int)'F' && display[1] == (int)'l') { status = 0x59; }
    else if(display[0] == (int)'L') { status = 0x5A; }
    else if(display[0] == (int)'C') { status = 0x5B; }
    else if(display[0] == (int)'G' && display[4] == (int)'B') { status = 0x5C; }
    else if(display[0] == (int)'S' && display[5] == (int)'B') { status = 0x5D; }
    else if(display[0] == (int)'S' && display[2] == (int)'a') { status = 0x5F; }
    else if(display[0] == (int)'P' && display[1] == (int)'r') { status = 0x60; }
    else { status = 0x07; }


    rc_puls = ecuBuffer[40];
    rc_puls += ecuBuffer[41] * 256;


    throttle = ecuBuffer[44] / 2.55;
    */

    //TODO Validate values
    sensorEGT->value = ecuBuffer[45] * 4;
    sensorRPM->value = (ecuBuffer[48] + (ecuBuffer[49] * 0x100)) * 100;
    sensorCurrent->value = (ecuBuffer[38] + (ecuBuffer[37] * 0x100)) / 100;
    sensorBattVoltage->value = ecuBuffer[46] * 6;
    sensorPumpVoltage->value = ecuBuffer[42] * 6;
}

