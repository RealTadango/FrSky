#ifndef SPORTHUB_H
    #define SPORTHUB_H

    #include <SPort.h>
    #include <Arduino.h>
    #include <SoftwareSerial.h>

class SPortHub {
        public:
    #ifdef Serial_
            SPortHub(int physicalId, Serial_& serial);
    #elif HardwareSerial
            SPortHub(int physicalId, HardwareSerial& serial);
    #endif
            SPortHub(int physicalId, int softwarePin);
            void begin();
            void handle();
            void registerSensor(SPortSensor& sensor);
            void sendCommand(int prim, int applicationId, int value);
            void (*commandReceived)(int prim, int applicationId, int value);
            int commandId;
        private:
            void SendData(sportData data, int prim);
            void SendByte(byte b);
            bool SendCommand();
            bool SendSensor();
            byte GetChecksum(byte data[], int start, int len);

    #ifdef Serial_
            Serial_* _hwStream;
    #elif HardwareSerial
            HardwareSerial* _hwStream;
    #endif
            SoftwareSerial* _swStream;
            SPortSensor** _sensors;
            int _sensorCount;
            int _sensorIndex;
            int _physicalId;
            int _softwarePin;
            bool _valid;
            short _index;
            byte _buffer[10];
            int _commandPrim;
            sportData _commandData;
    };
    
#endif