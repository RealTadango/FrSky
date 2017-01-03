#include <SoftwareSerial.h>

/* Powerbox M-Link to S.Port converter */

//#define DEBUG
#define TIMEOUT 5000

#define SPORT_IN 3
#define SPORT_OUT 3

#define SPORT_START 0x7E

#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00
//#define SPORT_HEADER_RESPONSE 0x32
//#define SPORT_HEADER_WORKING 0x20
//#define SPORT_HEADER_IDLE 0x21

#define SENSOR_PHYSICAL_ID 0x11

#define SENSOR_APPL_ID_RB1 0x0B00
#define SENSOR_APPL_ID_RB2 0x0B10

#define SENSOR_APPL_ID_GSPD 0x0830
#define SENSOR_APPL_ID_GALT 0x0820

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
int batt1Voltage, batt2Voltage = 0;
int batt1Amps, batt2Amps = 0;
int gpsSpeed, gpsAltitude, gpsDistance, gpsTravel = 0;

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
  long value = (frame[2] + (frame[3] * 256)) / 2;
  byte id = (frame[1] & 0xF0) / 16;


#if defined(DEBUG)
    debug.println("");    
    debug.print(id, DEC);
    debug.print(": ");
    debug.print(frame[1] & 0x0F, DEC);
    debug.print(", ");
    debug.print(value, DEC);
#endif


  if(id == 0x03)
  {
    batt1Voltage = value;
  }
  else if(id == 0x04)
  {
    batt2Voltage = value;
  }
  else if(id == 0x09)
  {
    gpsSpeed = value;
  }
  else if(id == 0x0A)
  {
    gpsAltitude = value;
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
    lh.longValue = (batt1Voltage * 100) + ((batt1Amps * 10) * 256);
  }
  else if(tele_counter == 1)
  {
    applID = SENSOR_APPL_ID_RB2;
    lh.longValue = (batt2Voltage * 100) + ((batt2Amps * 10) * 256);
  }
  else if(tele_counter == 2)
  {
    applID = SENSOR_APPL_ID_GSPD;
    lh.longValue = (gpsSpeed / 0.01852);
  }
  else if(tele_counter == 3)
  {
    applID = SENSOR_APPL_ID_GALT;
    lh.longValue = gpsAltitude * 100;
  }

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

  if(tele_counter >= 10)
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
