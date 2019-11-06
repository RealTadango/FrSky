#include <CustomSPortSensor.h>

CustomSPortSensor::CustomSPortSensor(sportData (*callback)(CustomSPortSensor*)) {
  _callback = callback;
}

sportData CustomSPortSensor::getData() {
  return _callback(this);
}