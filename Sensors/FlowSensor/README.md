# Flow Sensor
This sensor provides telemetry data for fuel flow from a flow sensor.

The sensor used for this meter can be found here: [BT Bio-Tech FCH-M Low Flow Flowmeter, Corrosive Chemicals, NPN](http://www.conrad.com/ce/en/product/155374/?insert=89&insertNoDeeplink&productname=BT-Bio-Tech-FCH-M-Low-Flow-Flowmeter-Corrosive-Chemicals-NPN)

Folder FlowSensor contains an Arduino sketch for a Atmega 328 type Arduino ((Pro) Mini, Nano ect). RB2 is connected to the pulse output of the sensor. RB3 is connected to the S.Port of the X-Series receiver.

The option PULSES_CFG in FlowSensor.ino is used to configure the sensor type. You calculate this value like this:
<pulses per L> / 1000 * 2

The above sensor has about 12.000 pulses (10.500 according to the specs) per L and every pulse is measures twice in the code code so a config value of 24 is correct. 

The following sensors ar available:
 - ml Used: Custom sensor with ID 0x5200
 - ml Flow: Custom sensor with ID 0x5210

