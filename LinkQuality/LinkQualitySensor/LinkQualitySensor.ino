#include <SoftwareSerial.h>
#define sequentialLossesThreshold 5
#define frameLossBuffer 100

#define SPORT_IN 3
#define SPORT_OUT 3

#define SPORT_START 0x7E
#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00

#define SENSOR_PHYSICAL_ID 0x12
#define SENSOR_APPL_ID 0x5100

byte buffer[25];
long index = 0;
byte previousValue = 0;
bool inFrame = false;
int sensorDelay = 0;

bool lastFrameLosses[frameLossBuffer];
int lastFrameLossesIndex = 0;
double currentpercentage = 100;

SoftwareSerial sport(SPORT_IN, SPORT_OUT, true);

//Helper for long / byte conversion
typedef union {
    char byteValue[4];
    long longValue;
} longHelper;

void setup() {
  //Sport UART 8N1 57K6
  sport.begin(57600);

  //Setup S.Bus capture on hardware serial
  Serial.begin(100000, SERIAL_8E2);

  //Switch to input, single wire mode
  pinMode(SPORT_IN, INPUT);
}

void loop() 
{
  //Read S.Port bytes
  while(sport.available())
  {
    byte val = sport.read();

    //Pass it to the handler
    NewValueSport(val);
  }
  
  while (Serial.available()) 
  {
    //Read MSB value from S.Bus stream
    byte value = Serial.read();

    //Only start a new frame if everything lines up correctly
    if(!inFrame && value == 0x0F && previousValue == 0x00)
    {
      index = 0;
      inFrame = true;
    }

    //When in frame, store the value in the buffer
    if(inFrame)
    {
      buffer[index] = value;
      index++;
    }

    //If all 25 bytes are received in the frame, handle it
    if(inFrame && index == 25)
    {
      inFrame = false; //Stop capturing
      handleFrame();
    }

    previousValue = value;
  }
}

void handleFrame()
{   
  bool framelost = buffer[23] & 0x04;

  lastFrameLosses[lastFrameLossesIndex] = framelost;
  lastFrameLossesIndex++;
  if(lastFrameLossesIndex >= frameLossBuffer)
  {
    lastFrameLossesIndex = 0;
  }
  
  int lastLost = 0;
  
  for(int i = 0; i < frameLossBuffer; i++)
  {
      if(lastFrameLosses[i])
      {
        lastLost++;
      }
  }

  currentpercentage = 100 - (((double)lastLost / (double)frameLossBuffer) * (double)100);
}

void handleSportFrame(byte frame[])
{
  //Not used for now
}

void sendFrame() {
  //Switch to output
  pinMode(SPORT_OUT, OUTPUT);  

  //Wait for output to be available
  delay(1);

  longHelper lh;
  lh.longValue = currentpercentage;

  byte frame[8];

  if (sensorDelay == 0)
  {
    frame[0] = SPORT_HEADER_DATA;    
    frame[1] = lowByte(SENSOR_APPL_ID);
    frame[2] = highByte(SENSOR_APPL_ID);
    frame[3] = lh.byteValue[0];
    frame[4] = lh.byteValue[1];
    frame[5] = lh.byteValue[2];
    frame[6] = lh.byteValue[3];
  }
  else
  {
    frame[0] = SPORT_HEADER_DISCARD;    
    frame[1] = 0;
    frame[2] = 0;
    frame[3] = 0;
    frame[4] = 0;
    frame[5] = 0;
    frame[6] = 0;
  }

  sensorDelay++;

  if(sensorDelay == 5)
  {
    sensorDelay = 0;
  }

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
