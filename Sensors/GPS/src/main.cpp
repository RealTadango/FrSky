#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPort.h>

//#define DEBUG
#define GPS_BAUD 38400

#define SENSOR_APPL_ID_GPS 0x0800
#define SENSOR_APPL_ID_GALT 0x0820
#define SENSOR_APPL_ID_GSPD 0x0830
#define SENSOR_APPL_ID_GCOURSE 0x0840
#define SENSOR_APPL_ID_GDATETIME 0x0850
#define SENSOR_APPL_ID_GDIST 0x5110
#define SENSOR_APPL_ID_GSATS 0x5111
#define SENSOR_APPL_ID_GHDOP 0x5112
#define SENSOR_APPL_ID_GTRAV 0x5120

TinyGPSPlus gps;
SPortHub hub(0x12, 3);
SimpleSPortSensor sensor_distance(SENSOR_APPL_ID_GDIST);
SimpleSPortSensor sensor_sats(SENSOR_APPL_ID_GSATS);
SimpleSPortSensor sensor_hdop(SENSOR_APPL_ID_GHDOP);
SimpleSPortSensor sensor_trav(SENSOR_APPL_ID_GTRAV);
SimpleSPortSensor sensor_altitude(SENSOR_APPL_ID_GALT);
SimpleSPortSensor sensor_speed(SENSOR_APPL_ID_GSPD);
SimpleSPortSensor sensor_course(SENSOR_APPL_ID_GCOURSE);
SimpleSPortSensor sensor_time(SENSOR_APPL_ID_GDATETIME);
SimpleSPortSensor sensor_date(SENSOR_APPL_ID_GDATETIME);
SimpleSPortSensor sensor_longitude(SENSOR_APPL_ID_GPS);
SimpleSPortSensor sensor_lattitude(SENSOR_APPL_ID_GPS);

unsigned long ledBlink = 0;
bool home_locked = false;
double home_long = 0;
double home_latt = 0;
double prev_long = 0;
double prev_latt = 0;
double traveled = 0;

void updateSensors();
void updateLed();

#ifdef DEBUG
SoftwareSerial debug(5, 5);
#endif

void setup() {
#ifdef DEBUG
  debug.begin(115200);
  pinMode(5, OUTPUT);
  debug.println("Start debug");
#endif

  Serial.begin(GPS_BAUD);

  hub.registerSensor(sensor_distance);
  hub.registerSensor(sensor_sats);
  hub.registerSensor(sensor_hdop);
  hub.registerSensor(sensor_trav);
  hub.registerSensor(sensor_altitude);
  hub.registerSensor(sensor_speed);
  hub.registerSensor(sensor_course);
  hub.registerSensor(sensor_time);
  hub.registerSensor(sensor_date);
  hub.registerSensor(sensor_longitude);
  hub.registerSensor(sensor_lattitude);
  hub.begin();

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  while (Serial.available() > 0) {
    if (gps.encode(Serial.read())) {
      updateSensors();
    }
  }

  hub.handle();
  updateLed();
}

void updateSensors() {
  double lng = gps.location.lng();
  double lat = gps.location.lat();
  double hdop = gps.hdop.hdop();

  if(!home_locked && gps.location.isValid()) {
    home_locked = true;
    prev_long = home_long = lng;
    prev_latt = home_latt = lat;
  }

  if(home_locked && gps.location.isValid()) {
    sensor_distance.value = gps.distanceBetween(home_latt, home_long, lat, lng);
    
    traveled += ((double)((int)(gps.distanceBetween(prev_latt, prev_long, lat, lng) * (double)10))) / (double)10;
    sensor_trav.value = traveled;

    prev_long = lng;
    prev_latt = lat;
  }

  sensor_sats.value = gps.satellites.value();
  sensor_hdop.value = hdop *100;
  sensor_altitude.value = gps.altitude.meters() * 100;
  sensor_speed.value = gps.speed.knots() * 1000;
  sensor_course.value = gps.course.deg() * 100;

  longHelper time;
  time.byteValue[0] = 0x00;
  time.byteValue[1] = (int8_t)gps.time.second();
  time.byteValue[2] = (int8_t)gps.time.minute();
  time.byteValue[3] = (int8_t)gps.time.hour();
  sensor_time.value = time.longValue;

  longHelper date;
  date.byteValue[0] = 0x01;
  date.byteValue[1] = (int8_t)gps.date.day();
  date.byteValue[2] = (int8_t)gps.date.month();
  date.byteValue[3] = (int8_t)(gps.date.year() - 2000);
  sensor_date.value = date.longValue;

  sensor_longitude.value = lng * 600000;
  if(sensor_longitude.value < 0) {
    bitSet(sensor_longitude.value, 30); //West
  } else {
    bitClear(sensor_longitude.value, 30); //East
  }
  bitSet(sensor_longitude.value, 31); //Longitude

  sensor_lattitude.value = lat * 600000;
  if(sensor_lattitude.value < 0) {
    bitSet(sensor_lattitude.value, 30); //South
  } else {
    bitClear(sensor_lattitude.value, 30); //North
  }
  bitClear(sensor_lattitude.value, 31); //Lattitude
}

void updateLed() {
  if(gps.charsProcessed() < 10) {
    digitalWrite(LED_BUILTIN, LOW);
  } else if(gps.location.isValid()) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (millis() >= (ledBlink + 300)) {
    ledBlink = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}