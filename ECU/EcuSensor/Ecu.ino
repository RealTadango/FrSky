#define ESC 0x1B

//Current index for receiving display byte
short ecu_index = 0;

//Previous value
byte ecu_prev;

//Repeat the enter key

//Byte received is valid for ECU display
bool ecu_valid = false;

void getTerminalFrame(byte data[])
{
  for(short pos = 0; pos <= 7; pos++)
  {
    //Check if this part of the display has changed
    if(display[pos * 4] != sent_display[pos * 4]
      || display[(pos * 4)+1] != sent_display[(pos * 4)+1]
      || display[(pos * 4)+2] != sent_display[(pos * 4)+2]
      || display[(pos * 4)+3] != sent_display[(pos * 4)+3])
    {
      //Update the sent display
      sent_display[pos * 4] = display[pos * 4];
      sent_display[(pos * 4)+1] = display[(pos * 4)+1];
      sent_display[(pos * 4)+2] = display[(pos * 4)+2];
      sent_display[(pos * 4)+3] = display[(pos * 4)+3];

      int applID = SENSOR_APPL_ID_TERMINAL + pos;

      //Prepare the S.Port data
      data[0] = SPORT_HEADER_DATA;
      data[1] = lowByte(applID);
      data[2] = highByte(applID);
      data[3] = display[pos * 4];
      data[4] = display[(pos * 4)+1];
      data[5] = display[(pos * 4)+2];
      data[6] = display[(pos * 4)+3];

      return;
    }
  }

  //Nothing to send, discard the frame
  data[0] = SPORT_HEADER_DISCARD;
  data[1] = 0x00;
  data[2] = 0x00;
  data[3] = 0x00;
  data[4] = 0x00;
  data[5] = 0x00;
  data[6] = 0x00;
}

#if defined(ECU_JETRONIC)
int enterKeyRepeat = 0;

void HandleEvojetFrame()
{
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
  bcdData[0] = display[5];
  bcdData[1] = display[4];
  bcdData[2] = display[3];

  egt = FromBCD(bcdData, 3);

  bcdData[0] = display[20];
  bcdData[1] = display[18];
  bcdData[2] = display[17];
  bcdData[3] = display[16];

  rpm = (uint32_t)FromBCD(bcdData, 4) * 100;
/*
  if(display[25] == (int)'U')
  {
    bcdData[0] = display[30];
    bcdData[1] = display[28];
    bcdData[2] = display[27];

    battVoltage = FromBCD(bcdData, 3) / (double)10;
  }

  if(display[25] == (int)'F')
  {
    bcdData[0] = display[30];
    bcdData[1] = display[29];
    bcdData[2] = display[27];

    pumpVoltage = FromBCD(bcdData, 3) / (double)100;
  }
*/
}

//Get a value from the BCD chars
int FromBCD(byte bcdData[], int length)
{
  int result = 0;

  for(int i = 0; i < length; i++)
  {
    if(bcdData[i] >= (int)'0' && bcdData[i] <= (int)'9')
    {
      int val = (bcdData[i] - (int)'0');

      if(i == 1)
      {
        val = val * 10;
      }
      else if(i == 2)
      {
        val = val * 100;
      }
      else if(i == 3)
      {
        val = val * 1000;
      }

      result += val;
    }
  }

  return result;
}

void SendKeyCode()
{
  if(key == 0 && enterKeyRepeat == 0)
  {
    return;
  }
  
  if(key == 1 || key == 2 || enterKeyRepeat > 0)
  {
    if(key == 1)
    {
      enterKeyRepeat = 15;  //Send 15 times the enter key
    }
    else if(enterKeyRepeat > 0)
    {
      enterKeyRepeat--;
    }

    //Enter
    ecu.write(0x7F);
  }
  else if(key == 3)
  {
    //Up
    ecu.write(0xBF);
  }
  else if(key == 4)
  {
    //Down
    ecu.write(0xDF);
  }

  key = 0;
}

void NewValueEcu(byte newVal)
{
  //Skip the esc command
  if(newVal == ESC)
  {
    ecu_valid = false;
  }
  //Request for keycode
  else if(ecu_prev == ESC && newVal == 0x4B)
  {
    delay(5);
    SendKeyCode();
    ecu_valid = false;
  }
  //Start of first LCD line
  else if(ecu_prev == 0x43 && newVal == 0x80)
  {
    ecu_index = 0;
    ecu_valid = true;
  }
  //Start of second LCD line
  else if(ecu_prev == 0x43 && newVal == 0xC0)
  {
    ecu_index = 16;
    ecu_valid = true;
  }
  //Store received byte when sequence matches
  else if(ecu_valid)
  {
    display[ecu_index] = newVal;

    //End of line one reached, disable storage
    if(ecu_index == 15)
    {
      ecu_valid = false;
    }
    //End of line two reached, disable storage and handle the full frame
    else if(ecu_index == 31)
    {
      ecu_valid = false;
      HandleEvojetFrame();
    }
    //Advance to the next position
    else
    {
      ecu_index++;
    }
  }

  ecu_prev = newVal;
}
#else if defined(ECU_FADEC)
int ecuBuffer[50];

void NewValueEcu(byte newVal)
{
  if(newVal == 253 && ecu_prev == 252)
  {  
    //New frame!
    ecu_index = 1;
    ecu_valid = true;
  }

  if(ecu_valid)
  {
    ecuBuffer[ecu_index] = newVal;

    if(ecu_index == 49)
    {
      ecu_valid = false;
      SendKeyCode();
      HandleXicoyFrame();
    }

    ecu_index++;
  }

  ecu_prev = newVal;
}

void SendKeyCode()
{
  if(key == 0)
  {
    return;
  }

  delay(2);

  //Setup key command
  byte data[26];
  data[0] = 0xDE;
  data[1] = 0xDF;
  data[2] = 0x70;

  //Reset all else to 0
  for(int i = 5; i < 25; i++)
  {
    data[i] = 0;
  }
  
  data[25] = 0xFF;

  if(key == 1) //data down
  {
    data[3] = 0x42;
    data[4] = 0xB2;
  }
  else if(key == 2) //data up
  {
    data[3] = 0x41;
    data[4] = 0xB1;
  }
  else if(key == 3) //menu up
  {
    data[3] = 0x43;
    data[4] = 0xB3;
  }
  else if(key == 4) //menu down
  {
    data[3] = 0x44;
    data[4] = 0xB4;
  }

  ecu.write(data, 26);

  key = 0;
}

void HandleXicoyFrame()
{
  //Led on indicates data from ECU
  digitalWrite(LED_BUILTIN, HIGH);

  int keyByte = 35;

  //Correct frame bytes
  int key = ecuBuffer[keyByte];

  for(int i = 2; i < 50; i++)
  {
    byte val = 255 - ecuBuffer[i] + key;

    if(val > 255)
    {
      val -= 255;
    }

    ecuBuffer[i] = val;
  }

  //Create display string
  for(int i = 0; i < 32; i++)
  {
    display[i] = ecuBuffer[i + 2];
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

  current = ecuBuffer[38];
  current += ecuBuffer[37] * 256;

  rc_puls = ecuBuffer[40];
  rc_puls += ecuBuffer[41] * 256;

  pumpVoltage = ecuBuffer[42] * 0.06;

  throttle = ecuBuffer[44] / 2.55;
  */

  egt = ecuBuffer[45] * 4;

  //battVoltage = ecuBuffer[46] * 0.06;

  rpm = ecuBuffer[48];
  rpm += ecuBuffer[49] * 256;
  rpm *= 100;
}
#endif

