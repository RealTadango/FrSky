#ifndef SPORT_H
    #define SPORT_H

    #define SPORT_BAUD 57600
    #define SPORT_START 0x7E
    #define SPORT_HEADER_DISCARD 0x00
    #define SPORT_HEADER_DATA 0x10
    #define SPORT_HEADER_READ 0x30
    #define SPORT_HEADER_WRITE 0x31
    #define SPORT_HEADER_RESPONSE 0x32

    #include <Arduino.h>
    #include <SoftwareSerial.h>

    typedef union {
        char byteValue[4];
        long longValue;
    } longHelper;

    struct sportData {
        long value = 0;
        int applicationId = 0;
    };

    class SPortSensor {
        public:
            void (*valueSend)(void);
            virtual sportData getData () = 0;
            bool enabled = true;
    };

    class CustomSPortSensor : public SPortSensor {
        public:
            CustomSPortSensor(sportData (*callback)(CustomSPortSensor*));
            virtual sportData getData ();
        private:
            sportData (*_callback)(CustomSPortSensor*);
    };

    class SimpleSPortSensor : public SPortSensor {
        public: 
            SimpleSPortSensor(int id);
            virtual sportData getData ();
            long value;
        private:
            int _id;
    };

    class SPortHub {
        public:
    #ifdef Serial_
            SPortHub(int physicalId, Serial_& serial);
    #else
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
    #else
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