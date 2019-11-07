#ifndef ECU_H
    #define ECU_H

    #include <Arduino.h>
    #include <SPort.h>

    class Ecu {
        public:
            byte terminalDisplay[32];
            int terminalKey;

            virtual void begin();
            virtual void registerSensors(SPortHub& hub);
            virtual void handle();
            virtual void enableSensors(bool enabled);
    };
#endif