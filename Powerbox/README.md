# Powerbox Sensor
This sensor provides telemetry data for Powerbox products

Folder PowerBoxSensor contains an Arduino sketch for a Atmega 328 type Arduino ((Pro) Mini, Nano ect). RX port is connected to the Powerbox telemetry output. RB3 is connected to the S.Port of the X-Series receiver.

This sensor maps the output from a Powerbox to FrSky sensor values. The powerbox must be set to input / telemetry type Futaba or S.Bus.

The following values / sensors are available:
 - Battery voltage. Voltage is with 0.1 precision for sensors RB1V and RB2V
 - Battery capacity left (when the powerbox supports it). This shows the mAh left as RB1C and RB2C
 - Receiver errors: 2 custom sensors with ID 0x5130 and 0x5140
 - Framelosses: Custom sensor with ID 0x5150
 - Holds: Custom sensor with ID 0x5160
 
When a GPS is attached to the powerbox the following sensors are availalble:
 - GSpd: GPS 3D speed
 - GAlt: GPS height
 - Distance to home: Custom sensor with id 0x5110 that shows the distance in meters
 - Distance traveled: Custom sensor with ID 0x5120 that shows the distance traveled in meters (100m precision)
 - Coordinates in lattitude, longitude
 
When a sensor is not availalble is it not tranismitted and cannot be discovered.
