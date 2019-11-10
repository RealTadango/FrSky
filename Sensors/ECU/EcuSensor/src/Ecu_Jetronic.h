#ifndef ECU_JETRONIC_H
  #define ECU_JETRONIC_H

  #include "Ecu.h"
  #include <SPort.h>

  class Ecu_Jetronic : public Ecu {
    public:
        byte terminalDisplay[32];
        int terminalKey;

        Ecu_Jetronic();
        void begin();
        void registerSensors(SPortHub& hub);
        void handle();
        void enableSensors(bool enabled);

    private:
        short ecuIndex = 0; //Current index for receiving display byte
        byte ecuPrev; //Previous value
        bool ecuValid = false; //Byte received is valid for ECU display
        int enterKeyRepeat = 0;

        SimpleSPortSensor* sensorEGT;
        SimpleSPortSensor* sensorRPM;
        SimpleSPortSensor* sensorBattVoltage;
        SimpleSPortSensor* sensorPumpVoltage;

        void SendKeyCode();
        int FromBCD(byte bcdData[], int length);
        void HandleEvojetFrame();
  };

#endif