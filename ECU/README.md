# ECU Sensor
This sensor provides telemetry data for EGT and RPM for Jetronic VX and Xicoy fadec ECU's

The EcuSensor folder contains an Arduino sketch for an Arduino mini. You can select the ECU type by #define ECU_JETRONIC or #define ECU_FADEC. RX and TX pins must be bridged with a diode for controlling the ECU from the sensor / TX. For only telemetry this is not needed. RB3 is connected to the S.Port of a X-series receiver.

The ECU folder contains a lua script for the FrSky Horus with OpenTX 2.2. This script shows the ECU terminal on the Horus.