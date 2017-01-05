# Link Qualoty Sensor
This sensor provides a link quality from S.Bus data.

Folder LinkQualitySensor contains an Arduino sketch for an Arduino Mini. RX port is connected to the S.Bus port of a FrSky receiver through an inverter. RB3 is connected to the S.Port of the X-Series receiver.

The sensor with id 0x5100 will show the link quality in %. This is not the RSSI but the actual quality of the received data.
