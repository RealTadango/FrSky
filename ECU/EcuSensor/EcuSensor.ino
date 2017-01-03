#include <SoftwareSerial.h>

#define DEBUG

//#define ECU_JETRONIC
#define ECU_FADEC

#define SPORT_IN 3
#define SPORT_OUT 3

#define SPORT_START 0x7E

#define SPORT_HEADER_DATA 0x10
#define SPORT_HEADER_DISCARD 0x00
//#define SPORT_HEADER_RESPONSE 0x32
//#define SPORT_HEADER_WORKING 0x20
//#define SPORT_HEADER_IDLE 0x21

#define SENSOR_PHYSICAL_ID 0x10

#define SENSOR_APPL_ID_TERMINAL 0x5000  
#define SENSOR_APPL_ID_EGT 0x0400  
#define SENSOR_APPL_ID_RPM 0x0500  

SoftwareSerial sport(SPORT_IN, SPORT_OUT, true);
#define ecu Serial

bool terminalMode = false;
byte key = 0, confirm = 0;
short tele_counter = 0;

//Ecu data
//Sent display values to compare
byte sent_display[32] = "                                ";

byte display[32] =      " FrSky ECU V1.0  By H.Kruisman  ";
                      //"12345678901234567890123456789012";
uint16_t egt;
uint32_t rpm;

//Sensor data for later implementation
//double battVoltage;
//uint16_t rc_puls;
//uint16_t current;
//double pumpVoltage;
//double throttle;
//byte status;

//Helper for long / byte conversion
typedef union {
    char byteValue[4];
    long longValue;
} longHelper;

void setup() {
  //Sport UART 8N1 57K6
  sport.begin(57600);

#if defined(ECU_JETRONIC)
  ecu.begin(9600);
#else if definec(ECU_FADEC)
  ecu.begin(4800);
#endif

#if defined(DEBUG)
  ecu.println("Start debug");
#endif  

  //Switch to input, single wire mode
  pinMode(SPORT_IN, INPUT);
}

void loop() {
  //Read S.Port bytes
  while(sport.available())
  {
    byte val = sport.read();

    //Pass it to the handler
    NewValueSport(val);
  }

  //Ecu handling
  while(ecu.available())
  {
    byte val = ecu.read();

    //Pass it to the handler
    NewValueEcu(val);
  }
}

void handleSportFrame(byte frame[])
{
  //Skip new command if old command has to be confirmed first
  if (confirm != 0)
  {
    return;
  }
  
  int prim = frame[2];
  int appId = frame[3] + (frame[4] * 256);

  if(appId == SENSOR_APPL_ID_TERMINAL && prim == 0x31) //ECU Terminal command
  {
    longHelper val;
    val.byteValue[0] = frame[5];
    val.byteValue[1] = frame[6];
    val.byteValue[2] = frame[7];
    val.byteValue[3] = frame[8];
    long cmd = val.longValue;

    confirm = cmd;

    if(cmd == 0x10)
    {
      //Enable terminal mode
      terminalMode = true;

      //Reset display buffer
      for(int i = 0; i <= 31; i++)
      {
        sent_display[i] = (char)' ';
      }
    }
    else if(cmd == 0x11)
    {
      //Disable terminal mode
      terminalMode = false;
    }
    else if(cmd >= 0x20)
    {
      //Write keyNumber to ECU key buffer
      key = cmd - 0x20;
    } 
  }
}

void getSensorFrame(byte data[])
{
  int applID = 0;
  longHelper lh;

  if(tele_counter == 0)
  {
    applID = SENSOR_APPL_ID_EGT;
    lh.longValue = egt;
  }
  else if(tele_counter == 1)
  {
    applID = SENSOR_APPL_ID_RPM;
    lh.longValue = rpm;
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

  //Send confirmation back for received command
  if(confirm != 0)
  {
    int applID = SENSOR_APPL_ID_TERMINAL;

    frame[0] = 0x32;
    frame[1] = lowByte(applID);
    frame[2] = highByte(applID);
    frame[3] = confirm;
    frame[4] = 0x00;
    frame[5] = 0x00;
    frame[6] = 0x00;
    
    confirm = 0;
  }
  //Send a terminal frame
  else if(terminalMode)
  {
    getTerminalFrame(frame);
  }
  //Sens a sensor frame
  else
  {
    getSensorFrame(frame);
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
