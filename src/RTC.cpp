#include "RTC.h"

// encode/decode data stored in RTC from binary to binary-coded-decimal and vice versa
static byte bcd2bin(byte val) { return val - 6 * (val >> 4); }
static byte bin2bcd(byte val) { return val + 6 * (val / 10); }

static byte read_i2c_register(byte addr, byte reg) {
  Wire.beginTransmission(addr);
  Wire.write((byte)reg);
  Wire.endTransmission();

  Wire.requestFrom(addr, (byte)1);
  return Wire.read();
}

static void write_i2c_register(byte addr, byte reg, byte val) {
  Wire.beginTransmission(addr);
  Wire.write((byte)reg);
  Wire.write((byte)val);
  Wire.endTransmission();
}

/*
* Disabling 32KHz clock on RTC saves 10mAh and greatly reduces power consumption
* while in sleep mode
* Not strictly needed to call every boot as the RTC will save the value in its
* registers, however if the external battery is removed it will be reset
*/
static void disable32K() {
  byte status = read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG);
  status &= ~(0x1 << 0x03);
  write_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, status);
}

void RTC::begin() {
  Wire.begin();
  disable32K();
}

void RTC::setSquareWave1HZ(bool state) {
  byte ctrl = read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL);

  ctrl &= ~0x04; // turn off INTCON
  ctrl &= ~0x18; // set freq bits to 0

  ctrl |= (state) ? 0x00 : 0x1C;  // 0x1C = 1hz, 00x0 = off
  write_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, ctrl);
}

void RTC::syncTime(byte *clockTime) {
  // set register
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_TIMEREG);  // set pointer to time register (0x00)
  Wire.endTransmission();

  // read from registers
  Wire.requestFrom(DS3231_ADDRESS, 7);
  clockTime[SECOND] = bcd2bin(Wire.read() & 0x7F);  // seconds
  clockTime[MINUTE] = bcd2bin(Wire.read());         // minutes
  clockTime[HOUR]   = bcd2bin(Wire.read());         // hours
  clockTime[WEEK]   = bcd2bin(Wire.read());         // day of the week
  clockTime[DAY]    = bcd2bin(Wire.read());         // days
  clockTime[MONTH]  = bcd2bin(Wire.read());         // months
  clockTime[YEAR]   = bcd2bin(Wire.read());         // years
}

void RTC::setTime(byte *clockTime) {
  // set register
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_TIMEREG); // set pointer to time register (0x00)

  // write to register
  Wire.write(bin2bcd(clockTime[SECOND])); // seconds
  Wire.write(bin2bcd(clockTime[MINUTE])); // minutes
  Wire.write(bin2bcd(clockTime[HOUR]));   // hours
  Wire.write(bin2bcd(clockTime[WEEK]));   // day of the week
  Wire.write(bin2bcd(clockTime[DAY]));    // days
  Wire.write(bin2bcd(clockTime[MONTH]));  // months
  Wire.write(bin2bcd(clockTime[YEAR]));   // years
  Wire.endTransmission();
}

byte RTC::getTemperature() {
  // set register
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_TEMPERATUREREG);  // set pointer to temperature register
  Wire.endTransmission();

  // read value from register
  Wire.requestFrom(DS3231_ADDRESS, 2);
  byte msb = Wire.read();
  Wire.read();

  return msb;
}
