#ifndef ECU_H
    #define ECU_H

    #include <Arduino.h>
    #include <SPort.h>

    class Ecu {
        public:
            char terminalDisplay[32] = " ECU Terminal    V1.0 for Frsky";
            int terminalKey;

            virtual void begin();
            virtual void registerSensors(SPortHub& hub);
            virtual void handle();
            virtual void enableSensors(bool enabled);
    };
#endif