#ifndef SPORT_H
    #define SPORT_H

    #define SPORT_BAUD 57600
    #define SPORT_START 0x7E
    #define SPORT_HEADER_DISCARD 0x00
    #define SPORT_HEADER_DATA 0x10
    #define SPORT_HEADER_READ 0x30
    #define SPORT_HEADER_WRITE 0x31
    #define SPORT_HEADER_RESPONSE 0x32

    typedef union {
        char byteValue[4];
        long longValue;
    } longHelper;

    struct sportData {
        long value = 0;
        int applicationId = 0;
    };

    #include <SPortSensor.h>
    #include <SimpleSPortSensor.h>
    #include <CustomSPortSensor.h>
    #include <SPortHub.h>

#endif