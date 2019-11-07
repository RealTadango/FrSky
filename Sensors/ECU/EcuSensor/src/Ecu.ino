#include <SPort.h>

class Ecu {
    public:
        void begin();
        void registerSensors(SPortHub& hub);
        void handle();
        void enableSensors(bool enabled);
}