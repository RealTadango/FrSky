#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SPort.h"

#ifdef Serial_
SPortHub::SPortHub(Serial_& serial) {
    _hwStream = &serial;
    _stream = &serial;
}
#else
SPortHub::SPortHub(HardwareSerial& serial) {
    _hwStream = &serial;
    _stream = &serial;
}
#endif

SPortHub::SPortHub(int softwarePin) {
    _swStream = &SoftwareSerial(softwarePin, softwarePin, true);
    _softwarePin = softwarePin;
    _stream = _swStream;
}

void SPortHub::begin() {
    if(_hwStream) {
        _hwStream->begin(SPORT_BAUD, SERIAL_8E2);
    } else if(_swStream) {
        _swStream->begin(SPORT_BAUD);
    }
}

void SPortHub::handle() {
    while(_stream->available()) {
        byte newByte = _stream->read();

        if(newByte == SPORT_START) {
            _valid = true;
            _index = 0;
        } else if(_index == 1) {
            int physicalID = newByte & 0x1F;

            for(int i = 0; i < MAX_SENSOR_COUNT; i++)
            {
                if(_sensors[i] != 0 && _sensors[i]->id == physicalID){
                    sensorData data = _sensors[i]->getValue();
                    SendData(data);
                    break;
                }
            }
        }

        if(_valid)
        {
            _buffer[_index] = newByte;
            _index++;
        }
    }
}

void SPortHub::registerSensor(SPortSensor& sensor) {
    for(int i = 0; i < MAX_SENSOR_COUNT; i++)
    {
        if(_sensors[i] == 0){
            _sensors[i] = &sensor;
            break;
        }
    }
}

SPortSensor::SPortSensor(int sensorId, sensorData (*getValueFunc)(void)) {
    id = sensorId;
    getData = getValueFunc;
}

sensorData SPortSensor::getValue() {
    return getData();
}

void SPortHub::SendData(sensorData data) {
    if(_swStream) {
        pinMode(_softwarePin, OUTPUT);
        delay(1);

    }

    byte frame[8];

    if(data.sensorId > 0) {
        longHelper lh;
        lh.longValue = data.value;
        frame[0] = SPORT_HEADER_DATA;    
        frame[1] = lowByte(data.sensorId);
        frame[2] = highByte(data.sensorId);
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

    frame[7] = GetChecksum(frame, 0, 7);

    //Send the frame
    for(short i = 0; i < 8; i++)
    {
        SendByte(frame[i]);
    }

    if(_swStream) {
        pinMode(_softwarePin, INPUT);
    }
}

byte SPortHub::GetChecksum(byte data[], int start, int len)
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
void SPortHub::SendByte(byte b)
{
  if(b == 0x7E)
  {
    _stream->write(0x7D);
    _stream->write(0x5E);
  }
  else if(b == 0x7D)
  {
    _stream->write(0x7D);
    _stream->write(0x5D);
  }
  else
  {
    _stream->write(b);
  }
}