// modified library from https://github.com/adafruit/RTClib
#ifndef RTC_h
#define RTC_h

#include <Arduino.h>
#include <Wire.h>

#define DS3231_ADDRESS 0x68   // I2C address for DS3231
#define DS3231_TIMEREG 0x00   // Time register
#define DS3231_CONTROL 0x0E   // Control register
#define DS3231_STATUSREG 0x0F // Status register
#define DS3231_TEMPERATUREREG  0x11 // Temperature register


#define SECOND  0
#define MINUTE  1
#define HOUR    2
#define DAY     3
#define MONTH   4
#define YEAR    5
#define WEEK    6

class RTC {
  public:
    void begin();

    void syncTime(byte *clockTime);
    void setTime(byte *clockTime);
    void setSquareWave1HZ(bool state);
    byte getTemperature();
};

#endif
