#include "LedMatrix.h"
#include "RTC.h"
#include <LowPower.h>


/*
  Binary Clock

  Display
    row 0 - TEMPERATURE
    row 1 - DAY OF THE WEEK
    row 2 - YEAR
    row 3 - MONTH
    row 4 - DAY
    row 5 - HOUR
    row 6 - MINUTE
    row 7 - SECOND
*/

// MAX7219 LED matrix
LedMatrix lm;

// DS3231 RTC
RTC rtc;

// global var to store time
byte clockTime[7];
#define SECOND  0
#define MINUTE  1
#define HOUR    2
#define DAY     3
#define MONTH   4
#define YEAR    5
#define WEEK    6

// buttons/switches/sensors
#define EDIT_SWITCH       3
#define INCREMENT_BUTTON  7
#define DECREMENT_BUTTON  5
#define EDIT_NEXT_BUTTON  6
#define LIGHT_SENSOR      A1

// interr-
#define CLOCK_PULSE_INTERRUPT 0 // D2
#define EDIT_SWITCH_INTERRUPT 1 // D3

// brightness value that will trigger the clock to go into deep sleep mode
#define LIGHT_SENSOR_PEAK_VALUE 512 // 0 - dark, 1024 - light

// variable used by interrupt
boolean edit_switch_activated = false;
void edit_switch_listener() {
  edit_switch_activated = true;
}

void sync_time();
void update_temperature();
void edit_time_mode();
bool light_sensor_peaked();
void deep_sleep_mode();
void update_temperature();
void update_clock_face();

// null function to attach clock pulse interrupt to, NULL dosent work
void nullFunction() {}

void setup() {
  // clock speed
  CLKPR = 0x80; // enable changing
  CLKPR = 0x04; // 1 MHz ( 16MHz / 16 (2^04) )

  // buttons/sensors/switches
  pinMode(EDIT_SWITCH, INPUT);
  pinMode(INCREMENT_BUTTON, INPUT);
  pinMode(DECREMENT_BUTTON, INPUT);
  pinMode(EDIT_NEXT_BUTTON, INPUT);
  pinMode(LIGHT_SENSOR, INPUT);

  // interrupts
  attachInterrupt(CLOCK_PULSE_INTERRUPT, nullFunction, FALLING);  // FALLING or RISING just not HIGH or LOW as they trigger every 500ms, no idea why
  attachInterrupt(EDIT_SWITCH_INTERRUPT, edit_switch_listener, HIGH);

  // RTC
  rtc.begin();
  rtc.setSquareWave1HZ(true); // Enable RTC square wave, connected to interrupt D2

  // sync time on boot from RTC & display temperature
  sync_time();
  update_temperature();
}

/*
* loop() is run whenever the arduino is woken up from sleep by an interrupt,
* either the edit mode switch or the RTC 1Hz square wave.
*/

unsigned int secondsSinceSync = 0; // max 2^16
void loop() {
  // check edit switch
  if (edit_switch_activated) {
    edit_time_mode();
  }
  incrementTimeBySecond();

  secondsSinceSync++; // will overflow back to 0
  if ((secondsSinceSync & 1023) == 0) { // 17m 4s 2^10
    check_light_sensor(); // go into deep sleep mode if light sensor is triggered
    if ((secondsSinceSync & 2047) == 0) { // 34m 8s 2^11
      sync_time(); // sync with RTC
      if ((secondsSinceSync & 4095) == 0) { // 68m 16s 2^12
        update_temperature(); // update temperature
      }
    }
  }

  // update led matrix with the time
  update_clock_face();

  // sleep waiting for external wakeup from interrupt pin
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}



void check_light_sensor() {
  if (light_sensor_peaked()) { // check LDR
    deep_sleep_mode();
  }
}

void incrementTimeBySecond() {
  // 9:59:59
  clockTime[SECOND]++; // 9:59:60
  if (clockTime[SECOND] >= 60) {
    clockTime[SECOND] = 0; // 9:59:00
    clockTime[MINUTE]++; // 9:60:00
    if (clockTime[MINUTE] >= 60) {
      clockTime[MINUTE] = 0; // 9:00:00
      clockTime[HOUR]++; // 10:00:00
      if (clockTime[HOUR] >= 24) {
        clockTime[HOUR] = 0;
        clockTime[DAY]++;

        // new month
        if (clockTime[DAY] >= getDaysInMonth()) {
          clockTime[DAY] = 0;
          clockTime[MONTH]++;
          if (clockTime[MONTH] >= 13) { // month starts at 1
            clockTime[MONTH] = 0;
            clockTime[YEAR]++;
          }
        }
        // increment weekday
        clockTime[WEEK]++;
        if (clockTime[WEEK] >= 8) { // week day starts at 1
          clockTime[WEEK] = 1;
        }
      }
    }
  }
}




void sync_time() {
  rtc.syncTime(clockTime);
}

void update_clock_face() {
  for (int i=1; i<8; i++) {
    lm.setRow(i, clockTime[7-i]);
  }
}

void update_temperature() {
  byte temp = rtc.getTemperature();
  lm.setRow(0, temp);
}

bool light_sensor_peaked() {
  return (analogRead(LIGHT_SENSOR) > LIGHT_SENSOR_PEAK_VALUE);
}

void deep_sleep_mode() {
  lm.shutdown(true);  // turn off LED Matrix
  rtc.setSquareWave1HZ(false); // stop RTC clock pulses

  // go into sleep mode, checking LDR every 2m 8s
  byte i; // a lil cheeky bit of optimisation
  while (light_sensor_peaked())
    for (i=0; i<16; i++)
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

  // wakeup
  lm.shutdown(false);
  rtc.setSquareWave1HZ(true);

  // resync time on wakeup
  sync_time();
}
