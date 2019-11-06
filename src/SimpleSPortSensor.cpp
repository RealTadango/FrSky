#include <SimpleSPortSensor.h>

SimpleSPortSensor::SimpleSPortSensor(int id) {
  _id = id;
  value = 0;
}

sportData SimpleSPortSensor::getData() {
    sportData data;
    data.applicationId = _id;
    data.value = value;
    return data;
}