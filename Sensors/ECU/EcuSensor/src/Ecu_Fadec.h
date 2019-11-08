#ifndef ECU_FADEC_H
  #define ECU_FADEC_H

  #include "Ecu.h"
  #include <SPort.h>

  class Ecu_Fadec : public Ecu {
    public:
      byte terminalDisplay[32];
      int terminalKey;

      Ecu_Fadec();
      void begin();
      void registerSensors(SPortHub& hub);
      void handle();
      void enableSensors(bool enabled);

    private:
      short ecuIndex = 0; //Current index for receiving display byte
      byte ecuPrev; //Previous value
      bool ecuValid = false; //Byte received is valid for ECU display
      int ecuBuffer[50];

      SimpleSPortSensor* sensorEGT;
      SimpleSPortSensor* sensorRPM;
      SimpleSPortSensor* sensorCurrent;
      SimpleSPortSensor* sensorBattVoltage;
      SimpleSPortSensor* sensorPumpVoltage;

      void SendKeyCode();
      void HandleXicoyFrame();
  };

#endif