#include <SoftwareSerial.h>

/* Powerbox M-Link to S.Port converter */

//#define DEBUG
#define TIMEOUT 5000

#define SPORT_IN 3
#define SPORT_OUT 3

#define SPORT_START 0x7E

#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00

#define SENSOR_PHYSICAL_ID 0x11

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

SoftwareSerial sport(SPORT_IN, SPORT_OUT, true);

#if defined(DEBUG)
SoftwareSerial debug(7,8);
#endif

short addressIndex = 0x00;
long lastTime = 0;
byte buffer[4];
byte buffer_index = 0;
short tele_counter = 0;

//Data holders
bool gps_enabled, usage_enabled = false;

int batt1Voltage, batt2Voltage = 0;
int batt1Left, batt2Left = 0;
int gpsSpeed, gpsAltitude, gpsDistance, gpsTravel = 0;
int rx1Errors, rx2Errors, rxFrameLoss, rxHolds = 0;
int longHigh, longLow, latHigh, latLow = 0;

//Helper for long / byte conversion
typedef union {
    char byteValue[4];
    long longValue;
} longHelper;

void setup() {
  //Sport UART 8N1 57K6
  sport.begin(57600);

  // put your setup code here, to run once:
  Serial.begin(38400);

  //Switch to input, single wire mode
  pinMode(SPORT_IN, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

#if defined(DEBUG)
  debug.begin(115200);
  debug.println("Debug start");
  debug.listen();
#endif
}

void loop() {
  //Read S.Port bytes
  while(sport.available())
  {
    byte val = sport.read();

    //Pass it to the handler
    NewValueSport(val);
  }

  //Send address request
  if(micros() > lastTime + TIMEOUT)
  {
    //Increase address
    addressIndex++;

    if(addressIndex > 0x0F)
    {
      addressIndex = 0;
    }

/*
#if defined(DEBUG)
    debug.println("");
    debug.print("Requesting sensor ");
    debug.print(addressIndex, HEX);
#endif
  */
    
    Serial.write(addressIndex);
    lastTime = micros();

    buffer_index = 0;
    
    //Clear buffer
    while(Serial.available())
    {
      Serial.read();
    }
  }
  else if(Serial.available())
  {
    if(buffer_index < 4)
    {
      buffer[buffer_index] = Serial.read();
      buffer_index++;

      if(buffer_index == 4)
      {
        //TODO Handle sensor data
        handleNewPBFrame(buffer);
      }
    }
  }
}

void handleNewPBFrame(byte frame[4])
{
  //Led on indicates data from powerbox
  digitalWrite(LED_BUILTIN, HIGH);
  
  long value = (frame[2] + (frame[3] * 0x100));
  byte id = (frame[1] & 0xF0) / 0x10;
  byte type = (frame[1] & 0x0F);

#if defined(DEBUG)
    debug.println("");    
    debug.print(id, DEC);
    debug.print(": ");
    debug.print(frame[1] & 0x0F, DEC);
    debug.print(", ");
    debug.print(value, DEC);
#endif

  if(id == 0x01)
  {
    rx1Errors = value;
  }
  else if(id == 0x02)
  {
    rx2Errors = value;
  }
  else if(id == 0x03)
  {
    batt1Voltage = value / 2;
  }
  else if(id == 0x04)
  {
    batt2Voltage = value  / 2;
  }
  else if(id == 0x05)
  {
    rxHolds = frame[2];
    rxFrameLoss = frame[3];
  }
  else if(id == 0x06 && type != 0)
  {
    batt1Left = value / 2;
    usage_enabled = true;    
  }
  else if(id == 0x07 && type != 0)
  {
    batt2Left = value / 2;
    usage_enabled = true;
  }
  else if(id == 0x08)
  {
    longHigh = value;
    gps_enabled = true;
  }
  else if(id == 0x09 && type != 0)
  {
    gpsSpeed = value / 2;
    gps_enabled = true;
  }
  else if(id == 0x0A && type != 0)
  {
    gpsAltitude = value / 2;
    gps_enabled = true;
  }
  else if(id == 0x0B && type != 0)
  {
    gpsDistance = value / 2;
    gps_enabled = true;
  }
  else if(id == 0x0C && type != 0)
  {
    gpsTravel = value / 2;
    gps_enabled = true;
  }
  else if(id == 0x0D)
  {
    longLow = value;
    gps_enabled = true;
  }
  else if(id == 0x0E)
  {
    latHigh = value;
    gps_enabled = true;
  }
  else if(id == 0x0F)
  {
    latLow = value;
    gps_enabled = true;
  }
}

void handleSportFrame(byte frame[])
{
  //Not used for now
}

void getSensorFrame(byte data[])
{
  int applID = 0;
  longHelper lh;

  if(tele_counter == 0)
  {
    applID = SENSOR_APPL_ID_RB1;
    lh.longValue = batt1Voltage * 100;
  }
  else if(tele_counter == 1)
  {
    applID = SENSOR_APPL_ID_RB2;
    lh.longValue = batt2Voltage * 100;
  }
  else if(tele_counter == 2 && usage_enabled)
  {
    applID = SENSOR_APPL_ID_RBMAH;
    lh.longValue = batt1Left + (batt2Left * 0x10000);
  }
  else if(tele_counter == 3 && gps_enabled)
  {
    applID = SENSOR_APPL_ID_GSPD;
    lh.longValue = (gpsSpeed / 0.01852);
  }
  else if(tele_counter == 4 && gps_enabled)
  {
    applID = SENSOR_APPL_ID_GALT;
    lh.longValue = gpsAltitude * 100;
  }
  else if(tele_counter == 5 && gps_enabled)
  {
    applID = SENSOR_APPL_ID_GDIST;
    lh.longValue = gpsDistance;
  }
  else if(tele_counter == 6 && gps_enabled)
  {
    applID = SENSOR_APPL_ID_GTRAV;
    lh.longValue = gpsTravel * 100;
  }
  else if(tele_counter == 7)
  {
    applID = SENSOR_APPL_ID_RX1ERR;
    lh.longValue = rx1Errors;
  }
  else if(tele_counter == 8)
  {
    applID = SENSOR_APPL_ID_RX2ERR;
    lh.longValue = rx2Errors;
  }
  else if(tele_counter == 9)
  {
    applID = SENSOR_APPL_ID_RXFLS;
    lh.longValue = rxFrameLoss;
  }
  else if(tele_counter == 10)
  {
    applID = SENSOR_APPL_ID_RXHLDS;
    lh.longValue = rxHolds;
  }
//  else if(tele_counter == 11)
//  {
//    applID = SENSOR_APPL_ID_GPS;
//
//    longHelper cv;
//    cv.byteValue[0] = highByte(latHigh);
//    cv.byteValue[1] = lowByte(latHigh);
//    cv.byteValue[2] = highByte(latLow);
//    cv.byteValue[3] = lowByte(latLow);
//    
//    long coor = cv.longValue;
//      
//    if(coor < 0)
//    {
//      coor = 0 - coor;          //Negative, make positive
//      bitClear(coor, 31);       //South
//      bitSet(coor, 30);
//    }
//    else
//    {
//      bitClear(coor, 31);       //North
//      bitClear(coor, 30);
//    }
//
//    lh.longValue = coor;
//  }
//  else if(tele_counter == 12)
//  {
//    applID = SENSOR_APPL_ID_GPS;
//
//    longHelper cv;
//    cv.byteValue[0] = highByte(longHigh);
//    cv.byteValue[1] = lowByte(longHigh);
//    cv.byteValue[2] = highByte(longLow);
//    cv.byteValue[3] = lowByte(longLow);
//    
//    long coor = cv.longValue;
//
//    if(coor < 0)
//    {
//      coor = 0 - coor;          //Negative, make positive
//      bitSet(coor, 31);         //West
//      bitSet(coor, 30);
//    }
//    else
//    {
//      bitSet(coor, 31);         //East
//      bitClear(coor, 30);
//    }
//
//    lh.longValue = coor;
//  }

  if(applID == 0)
  {
    data[0] = SPORT_HEADER_DISCARD;
  }
  else
  {
    data[0] = SPORT_HEADER_DATA;    
  }
  data[1] = lowByte(applID);
  data[2] = highByte(applID);
  data[3] = lh.byteValue[0];
  data[4] = lh.byteValue[1];
  data[5] = lh.byteValue[2];
  data[6] = lh.byteValue[3];

  if(tele_counter >= 12)
  {
    tele_counter = 0;
  }
  else
  {
    tele_counter++;
  }
}

void sendFrame() {
  //Switch to output
  pinMode(SPORT_OUT, OUTPUT);  

  //Wait for output to be available
  delay(1);

  byte frame[8];

  getSensorFrame(frame);

  //Calculate checksum for the frame
  frame[7] = getChecksum(frame, 0, 7);

  //Send the frame
  for(short i = 0; i < 8; i++)
  {
    sendByte(frame[i]);
  }

  //Switch back to input
  pinMode(SPORT_IN, INPUT);  
}
