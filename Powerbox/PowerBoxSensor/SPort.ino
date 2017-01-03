//Frame buffer
byte sport_buffer[10];

//Frame index
short sport_index;

//Receiving valid frame data
bool sport_valid = false;

void NewValueSport(byte val)
{
  if(val == SPORT_START)
  {
    //Start S.Port frame capture
    sport_valid = true;
    sport_index = 0;
  }

  //Get physical id without CRC bits
  byte physicalID = val & 0x1F;

  //If the second byte is the sensor id it is a request for my sensor
  if(sport_index == 1 && physicalID == SENSOR_PHYSICAL_ID)
  {
      //Stop capturing
      sport_valid = false;
      
      //Needs my data so send it
      sendFrame();
  }

  //If we are capturing the sport data, store it in the buffer
  if(sport_valid)
  {
    sport_buffer[sport_index] = val;
    sport_index++;

    //Complete frame received, handle it
    if(sport_index >= 10)
    {
      sport_valid = false;

      //Get checksum for 7 bytes, prim, application id(2) and data(4)
      byte checksum = getChecksum(sport_buffer, 2, 7);

      //Handle frame if checksum matches
      if(checksum == sport_buffer[9])
      {
        handleSportFrame(sport_buffer);
      }
    }
  }
}

//Calculate checksum for the frame, refactor later :)
byte getChecksum(byte data[], int start, int len)
{
  long total = 0;

  for(int i = start; i < (start + len); i++)
  {
    total += data[i];
  }

  if(total >= 0x700)
  {
    total+= 7;
  }
  else if(total >= 0x600)
  {
    total+= 6;
  }
  else if(total >= 0x500)
  {
    total+= 5;
  }
  else if(total >= 0x400)
  {
    total+= 4;
  }
  else if(total >= 0x300)
  {
    total+= 3;
  }
  else if(total >= 0x200)
  {
    total+= 2;
  }
  else if(total >= 0x100)
  {
    total++;
  }

  return 0xFF - total;
}

//Send a data byte the FrSky way
void sendByte(byte b)
{
  if(b == 0x7E)
  {
    sport.write(0x7D);
    sport.write(0x5E);
  }
  else if(b == 0x7D)
  {
    sport.write(0x7D);
    sport.write(0x5D);
  }
  else
  {
    sport.write(b);
  }
}

