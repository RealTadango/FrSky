#include <SoftwareSerial.h>
#include <EEPROM.h>

//#define DEBUG

#define SPORT_IN 3
#define SPORT_OUT 3

#define PULSES_IN 2
#define SPORT_START 0x7E

#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00

#define SENSOR_PHYSICAL_ID 0x12

#define SENSOR_CONF_PULSES  0x5290
#define SENSOR_CONF_TANKSIZE 0x5291

#define SENSOR_APPL_ID_USED 0x5200 
#define SENSOR_APPL_ID_LEFT 0x5201 
#define SENSOR_APPL_ID_CURR 0x5210  
  

SoftwareSerial sport(SPORT_IN, SPORT_OUT, true);

short tele_counter = 0;
long count_raw = 0;
long count_last = 0;

//This is the pulses per ml * 2
double pulses_cfg = 0;
int tank_size = 0;
long lastTime = 0;

uint16_t fuelUsed;
uint16_t fuelCurrent;
uint16_t fuelLeft;

int send_config = 0;

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

  loadData();
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

  fuelUsed = (uint16_t)(count_raw / pulses_cfg);

  if(fuelUsed >= tank_size)
  {
    fuelLeft = 0;
  }
  else
  {
    fuelLeft = tank_size - fuelUsed;
  }
  
  if (millis() > lastTime + 2000)
  {
    int diff = millis() - lastTime;
    long timeCount = (count_raw - count_last);

    //calc fuel flow in ml/min
    fuelCurrent = (double)timeCount / pulses_cfg * 60 / ((double)diff / 1000);
    
    lastTime = millis();
    count_last = count_raw;
  }
}

void handleSportFrame(byte frame[])
{
  int prim = frame[2];
  int appId = frame[3] + (frame[4] * 256);

  if(prim == 0x31 && appId == SENSOR_CONF_PULSES)
  {
    send_config = 2;
  }
  else if(prim == 0x32)
  {
    longHelper val;
    val.byteValue[0] = frame[5];
    val.byteValue[1] = frame[6];
    val.byteValue[2] = frame[7];
    val.byteValue[3] = frame[8];
      
    if(appId == SENSOR_CONF_PULSES)
    {
      pulses_cfg = val.longValue / (double)100;

    }
    else if(appId == SENSOR_CONF_TANKSIZE)
    {
      tank_size = val.longValue;
    }

    saveData();
  }
}

void getSensorFrame(byte data[])
{
  int applID = 0;
  longHelper lh;

  if(send_config > 0)
  {
    //Send config data
    if(send_config == 2)
    {
      applID = 0x5000; //Write to OpenTX Buffer
      lh.longValue = pulses_cfg * 100;
      send_config = 1;
    }
    else if(send_config == 1)
    {
      applID = 0x5001; //Write to OpenTX Buffer
      lh.longValue = tank_size;
      send_config = 0;
    }
  }
  else
  {
    //Send sensor data
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
    else if(tele_counter == 2)
    {
      applID = SENSOR_APPL_ID_LEFT;
      lh.longValue = fuelLeft;
    }
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

void writeEEPROMValue(int pos, int value)
{
  EEPROM.write(pos * 2, lowByte(value));
  EEPROM.write((pos * 2) + 1, highByte(value));
}

int readEEPROMValue(int pos, int value)
{
  if(EEPROM.read(pos * 2) == 255 && EEPROM.read((pos * 2) + 1) == 255)
  {
    return value;
  }
  else
  {
    return EEPROM.read(pos * 2) + (EEPROM.read((pos * 2) + 1) * 256);
  }
}

void saveData()
{
  writeEEPROMValue(0, pulses_cfg * 100);
  writeEEPROMValue(1, tank_size);
}

void loadData()
{
  //default = 5.70 For B.I.O-TECH e.K. FCH-M-POM-LC (G 1/8) WITHOUT the restrictor
  //at 400 ml/s average
  pulses_cfg = readEEPROMValue(0, 570) / (double)100;
  tank_size = readEEPROMValue(1, 1000); //Default tank size to 1000
}
