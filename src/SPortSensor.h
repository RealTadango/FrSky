#ifndef SPORTSENSOR_H
    #define SPORTSENSOR_H

    #include <SPort.h>

    class SPortSensor {
        public:
            void (*valueSend)(void);
            virtual sportData getData () = 0;
            bool enabled = true;
    };

#endif