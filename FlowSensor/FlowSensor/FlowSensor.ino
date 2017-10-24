#include <SoftwareSerial.h>

//#define DEBUG

#define SPORT_IN 3
#define SPORT_OUT 3

#define PULSES_IN 2
//This is the pulses per ml * 2
#define PULSES_CFG 5.33	//For B.I.O-TECH e.K. FCH-M-POM-LC (G 1/8) WITHOUT the restrictor

#define SPORT_START 0x7E

#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00

#define SENSOR_PHYSICAL_ID 0x12

#define SENSOR_APPL_ID_USED 0x5200  
#define SENSOR_APPL_ID_CURR 0x5210  

SoftwareSerial sport(SPORT_IN, SPORT_OUT, true);

short tele_counter = 0;
long count_raw = 0;
long count_last = 0;

long lastTime = 0;

uint16_t fuelUsed;
uint16_t fuelCurrent;

//Helper for long / byte conversion
typedef union {
    char byteValue[4];
    long longValue;
} longHelper;

void setup() {
  //Sport UART 8N1 57K6
  sport.begin(57600);

#if defined(DEBUG)
  Serial.begin(115200);
  Serial.println("Start debug");
#endif  

  //Switch to input, single wire mode
  pinMode(SPORT_IN, INPUT);

  //Sensor input
  pinMode(PULSES_IN, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  attachInterrupt(digitalPinToInterrupt(PULSES_IN), flowTick, CHANGE);
}

void flowTick()
{
  count_raw++;
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  //Read S.Port bytes
  while(sport.available())
  {
    byte val = sport.read();

    //Pass it to the handler
    NewValueSport(val);
  }

  fuelUsed = (uint16_t)(count_raw / PULSES_CFG);

  if (millis() > lastTime + 2000)
  {
    int diff = millis() - lastTime;
    long timeCount = (count_raw - count_last);

    //calc fuel flow in ml/min
    fuelCurrent = (double)timeCount / PULSES_CFG * 60 / ((double)diff / 1000);
    
    lastTime = millis();
    count_last = count_raw;
  }
}

void handleSportFrame(byte frame[])
{
  //No configuration system for now
}

void getSensorFrame(byte data[])
{
  int applID = 0;
  longHelper lh;

  if(tele_counter == 0)
  {
    applID = SENSOR_APPL_ID_USED;
    lh.longValue = fuelUsed;
  }
  else if(tele_counter == 1)
  {
    applID = SENSOR_APPL_ID_CURR;
    lh.longValue = fuelCurrent;
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
