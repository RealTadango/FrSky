#include <SPortHub.h>

#ifdef Serial_
SPortHub::SPortHub(int physicalId, Serial_& serial):
  _physicalId(physicalId),
  _hwStream(&serial),
  _sensorIndex(0) {
}
#elif HardwareSerial
SPortHub::SPortHub(int physicalId, HardwareSerial& serial):
  _physicalId(physicalId),
  _hwStream(&serial),
  _sensorIndex(0) {
}
#endif

SPortHub::SPortHub(int physicalId, int softwarePin):
  _physicalId(physicalId),
  _softwarePin(softwarePin),
  _sensorIndex(0) {
    _swStream = new SoftwareSerial(_softwarePin, _softwarePin, true);
}

void SPortHub::begin() {
  #ifdef _hwStream
    if(_hwStream) {
      _hwStream->begin(SPORT_BAUD, SERIAL_8E2);
    } else {
      _swStream->begin(SPORT_BAUD);
    }
  #else
      _swStream->begin(SPORT_BAUD);
  #endif
}

bool SPortHub::SendCommand() {
  if(_commandPrim > 0) {
    SendData(_commandData, _commandPrim);
    _commandPrim = 0;
    return true;
  }

  return false;
}

bool SPortHub::SendSensor() {
  if(_sensorCount == 0) {
    return false;
  }

  int startIndex = _sensorIndex; // Know where we started to end the loop

  do {
    SPortSensor *sensor = _sensors[_sensorIndex];

    _sensorIndex++; // Set next index for previous sensor check
    if (_sensorIndex >= _sensorCount) {
      _sensorIndex = 0;
    }

    if(sensor->enabled) {
      sportData data = sensor->getData();

      if(data.applicationId > 0) {
        SendData(data, SPORT_HEADER_DATA);
        
        if(sensor->valueSend) {
          sensor->valueSend();
        }

        return true; //Data send, end loop
      } 
    }
  }
  while(_sensorIndex != startIndex); // Check entire array

  return false; // No active sensor
}

void SPortHub::handle() {
  #ifdef _hwStream
    Stream* stream = _hwStream ? (Stream*)_hwStream : (Stream*)_swStream;
  #else
    Stream* stream = _swStream;
  #endif
    while(stream->available() > 0) {
        byte newByte = stream->read();

        if(newByte == SPORT_START) {
          _valid = true;
          _index = 0;
        } else if(_valid && _index == 1) {
          //Check if the frame / request is for us
          int physicalID = newByte & 0x1F;

          if(_physicalId == physicalID && stream->available() == 0) {
            if(!SendCommand()) {
              _valid = !SendSensor();
            }
          } else if(commandId != physicalID) {
            //Other ID or to late
            _valid = false;
          }
        }

        if(_valid) {
          _buffer[_index] = newByte;
          _index++;

          if(_index >= 10)
          {
            _valid = false;
            if(commandReceived) {
              int applicationId = _buffer[3] + (_buffer[4] * 256);
              longHelper lh;
              lh.byteValue[0] = _buffer[5];
              lh.byteValue[1] = _buffer[6];
              lh.byteValue[2] = _buffer[7];
              lh.byteValue[3] = _buffer[8];
              commandReceived(_buffer[2], applicationId, lh.longValue);
            }
          }
        }
    }
}

void SPortHub::registerSensor(SPortSensor& sensor) {
    SPortSensor** newSensors = new SPortSensor*[_sensorCount + 1];

    for(int i = 0; i < _sensorCount; i++) {
      newSensors[i] = _sensors[i];
    }
    newSensors[_sensorCount] = &sensor;
    
    if(_sensors != nullptr) {
      free(_sensors);
    }
    _sensors = newSensors;
    _sensorCount++;
}

void SPortHub::sendCommand(int prim, int applicationId, int value)
{
  _commandPrim = prim;
  _commandData.applicationId = applicationId;
  _commandData.value = value;
}

void SPortHub::SendData(sportData data, int prim) {
    if(_swStream) {
        pinMode(_softwarePin, OUTPUT);
        delay(1);
    }

    longHelper lh;
    lh.longValue = data.value;

    byte frame[8];
    frame[0] = prim;    
    frame[1] = lowByte(data.applicationId);
    frame[2] = highByte(data.applicationId);
    frame[3] = lh.byteValue[0];
    frame[4] = lh.byteValue[1];
    frame[5] = lh.byteValue[2];
    frame[6] = lh.byteValue[3];
    frame[7] = GetChecksum(frame, 0, 7);

    //Send the frame
    for(short i = 0; i < 8; i++) {
        SendByte(frame[i]);
    }

    if(_swStream) {
        pinMode(_softwarePin, INPUT);
    }
}

byte SPortHub::GetChecksum(byte data[], int start, int len) {
  long total = 0;

  for(int i = start; i < (start + len); i++) {
    total += data[i];
  }

  if(total >= 0x700) {
    total+= 7;
  } else if(total >= 0x600) {
    total+= 6;
  } else if(total >= 0x500) {
    total+= 5;
  } else if(total >= 0x400) {
    total+= 4;
  } else if(total >= 0x300) {
    total+= 3;
  } else if(total >= 0x200) {
    total+= 2;
  } else if(total >= 0x100) {
    total++;
  }

  return 0xFF - total;
}

//Send a data byte the FrSky way
void SPortHub::SendByte(byte b) {
  #ifdef _hwStream
    Stream* stream = _hwStream ? (Stream*)_hwStream : (Stream*)_swStream;
  #else
    Stream* stream = _swStream;
  #endif

  if(b == 0x7E) {
    stream->write(0x7D);
    stream->write(0x5E);
  } else if(b == 0x7D) {
    stream->write(0x7D);
    stream->write(0x5D);
  } else {
    stream->write(b);
  }
}