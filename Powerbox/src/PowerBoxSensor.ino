#include <Sport.h>

/* Powerbox M-Link to S.Port converter */

#define SPORT_PIN 3
#define SPORT_PHYSICAL_ID 0x11

#define TIMEOUT 5000

#define SENSOR_APPL_ID_RB1 0x0B00
#define SENSOR_APPL_ID_RB2 0x0B10
#define SENSOR_APPL_ID_RBMAH 0x0B30
#define SENSOR_APPL_ID_RX1ERR 0x5130
#define SENSOR_APPL_ID_RX2ERR 0x5140
#define SENSOR_APPL_ID_RXFLS 0x5150
#define SENSOR_APPL_ID_RXHLDS 0x5160

#define SENSOR_APPL_ID_GPS 0x0800
#define SENSOR_APPL_ID_GSPD 0x0830
#define SENSOR_APPL_ID_GALT 0x0820
#define SENSOR_APPL_ID_GDIST 0x5110
#define SENSOR_APPL_ID_GTRAV 0x5120

sportData getSportData(CustomSPortSensor* sensor);

SPortHub hub(SPORT_PHYSICAL_ID, SPORT_PIN);
CustomSPortSensor dynamicSensor(getSportData);

short addressIndex = 0x00;
long lastTime = 0;
byte buffer[4];
byte buffer_index = 0;
short tele_counter = 0;

//Data holders
bool pb_enabled, gps_enabled, usage_enabled = false;

int batt1Voltage, batt2Voltage = 0;
int batt1Left, batt2Left = 0;
int gpsSpeed, gpsAltitude, gpsDistance, gpsTravel = 0;
int rx1Errors, rx2Errors, rxFrameLoss, rxHolds = 0;
int longHigh, longLow, latHigh, latLow = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);

  hub.registerSensor(dynamicSensor);
  hub.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  //Send address request
  if(micros() > lastTime + TIMEOUT) {
    //Increase address
    addressIndex++;

    if(addressIndex > 0x0F) {
      addressIndex = 0;
    }

    Serial.write(addressIndex);
    lastTime = micros();

    buffer_index = 0;
    
    //Clear buffer
    while(Serial.available()) {
      Serial.read();
    }
  } else if(Serial.available()) {
    if(buffer_index < 4) {
      buffer[buffer_index] = Serial.read();
      buffer_index++;

      if(buffer_index == 4) {
        //TODO Handle sensor data
        handleNewPBFrame(buffer);
      }
    }
  }
}

void handleNewPBFrame(byte frame[4]) {
  //Led on indicates data from powerbox
  digitalWrite(LED_BUILTIN, HIGH);
  
  long value = (frame[2] + (frame[3] * 0x100));
  byte id = (frame[1] & 0xF0) / 0x10;
  byte type = (frame[1] & 0x0F);

  if(id == 0x01) {
    rx1Errors = value;
    pb_enabled = true;
  } else if(id == 0x02) {
    rx2Errors = value;
    pb_enabled = true;
  } else if(id == 0x03) {
    batt1Voltage = value / 2;
    pb_enabled = true;
  } else if(id == 0x04) {
    batt2Voltage = value  / 2;
    pb_enabled = true;
  } else if(id == 0x05) {
    rxHolds = frame[2];
    rxFrameLoss = frame[3];
    pb_enabled = true;
  } else if(id == 0x06 && type != 0) {
    batt1Left = value / 2;
    usage_enabled = true;    
  } else if(id == 0x07 && type != 0) {
    batt2Left = value / 2;
    usage_enabled = true;
  } else if(id == 0x08) {
    longHigh = value;
    gps_enabled = true;
  } else if(id == 0x09 && type != 0) {
    gpsSpeed = value / 2;
    gps_enabled = true;
  } else if(id == 0x0A && type != 0) {
    gpsAltitude = value / 2;
    gps_enabled = true;
  } else if(id == 0x0B && type != 0) {
    gpsDistance = value / 2;
    gps_enabled = true;
  } else if(id == 0x0C && type != 0) {
    gpsTravel = value / 2;
    gps_enabled = true;
  } else if(id == 0x0D) {
    longLow = value;
    gps_enabled = true;
  } else if(id == 0x0E) {
    latHigh = value;
    gps_enabled = true;
  } else if(id == 0x0F) {
    latLow = value;
    gps_enabled = true;
  }
}

sportData getSportData(CustomSPortSensor* sensor) {
  sportData data;

  if(tele_counter == 0 && pb_enabled) {
    data.applicationId = SENSOR_APPL_ID_RB1;
    data.value = batt1Voltage * 100;
  } else if(tele_counter == 1 && pb_enabled) {
    data.applicationId = SENSOR_APPL_ID_RB2;
    data.value = batt2Voltage * 100;
  } else if(tele_counter == 2 && usage_enabled) {
    data.applicationId = SENSOR_APPL_ID_RBMAH;
    data.value = batt1Left + (batt2Left * 0x10000);
  } else if(tele_counter == 3 && gps_enabled) {
    data.applicationId = SENSOR_APPL_ID_GSPD;
    data.value = (gpsSpeed / 0.01852);
  } else if(tele_counter == 4 && gps_enabled) {
    data.applicationId = SENSOR_APPL_ID_GALT;
    data.value = gpsAltitude * 100;
  } else if(tele_counter == 5 && gps_enabled) {
    data.applicationId = SENSOR_APPL_ID_GDIST;
    data.value = gpsDistance;
  } else if(tele_counter == 6 && gps_enabled) {
    data.applicationId = SENSOR_APPL_ID_GTRAV;
    data.value = gpsTravel * 100;
  } else if(tele_counter == 7 && pb_enabled) {
    data.applicationId = SENSOR_APPL_ID_RX1ERR;
    data.value = rx1Errors;
  } else if(tele_counter == 8 && pb_enabled) {
    data.applicationId = SENSOR_APPL_ID_RX2ERR;
    data.value = rx2Errors;
  } else if(tele_counter == 9 && pb_enabled) {
    data.applicationId = SENSOR_APPL_ID_RXFLS;
    data.value = rxFrameLoss;
  } else if(tele_counter == 10 && pb_enabled) {
    data.applicationId = SENSOR_APPL_ID_RXHLDS;
    data.value = rxHolds;
  } else if(tele_counter == 11 && gps_enabled) {
    data.applicationId = SENSOR_APPL_ID_GPS;

    longHelper cv;
    cv.byteValue[3] = highByte(latHigh);
    cv.byteValue[2] = lowByte(latHigh);
    cv.byteValue[1] = highByte(latLow);
    cv.byteValue[0] = lowByte(latLow);
    
    long coor = cv.longValue / 100 * 6;
      
    if(coor < 0) {
      coor = 0 - coor;          //Negative, make positive
      bitClear(coor, 31);       //South
      bitSet(coor, 30);
    } else {
      bitClear(coor, 31);       //North
      bitClear(coor, 30);
    }

    data.value = coor;
  } else if(tele_counter == 12 && gps_enabled) {
    data.applicationId = SENSOR_APPL_ID_GPS;

    longHelper cv;
    cv.byteValue[3] = highByte(longHigh);
    cv.byteValue[2] = lowByte(longHigh);
    cv.byteValue[1] = highByte(longLow);
    cv.byteValue[0] = lowByte(longLow);
    
    long coor = cv.longValue / 100 * 6;

    if(coor < 0) {
      coor = 0 - coor;          //Negative, make positive
      bitSet(coor, 31);         //West
      bitSet(coor, 30);
    } else {
      bitSet(coor, 31);         //East
      bitClear(coor, 30);
    }

    data.value = coor;
  }

  if(tele_counter >= 12) {
    tele_counter = 0;
  } else {
    tele_counter++;
  }

  return data;
}