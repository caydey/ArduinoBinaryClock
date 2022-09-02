
const byte timeBounds[7][2] PROGMEM = {
  {0, 59},  // seconds
  {0, 59},  // minutes
  {0, 23},  // hours
  {0, 0},   // days (controlled by getDaysInMonth())
  {1, 12},  // months
  {0, 63},  // years
  {1, 7},   // weeks
};

void edit_time_mode() {
  // start editing seconds row
  byte editingRow = 7;  // 1 week, 2 second, 3 minute, 4 hour, 5 day, 6 month, 7 year
  highlight_row(7); // highlight editing row (seconds)

  int loopCount = 0;
  bool timeEdited = false;
  while (digitalRead(EDIT_SWITCH)) {  // edit switch on
    LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF); // delay

    if (digitalRead(EDIT_NEXT_BUTTON)) {  // edit next button
      unhighlight_row(editingRow);  // unhighlight prev row
      if (--editingRow == 0) editingRow = 7;  // move up
      highlight_row(editingRow);  // highlight row
    } else {  // increment or decrement button
      bool incrementButtonState = digitalRead(INCREMENT_BUTTON);
      if (incrementButtonState || digitalRead(DECREMENT_BUTTON)) {
        timeEdited = true;

        if (editingRow == 4) { // DAY
          edit_clock_row_value(editingRow, 1, getDaysInMonth(), incrementButtonState);
        } else {
          edit_clock_row_value(editingRow, pgm_read_byte(&timeBounds[7-editingRow][0]), pgm_read_byte(&timeBounds[7-editingRow][1]), incrementButtonState);
          /*
          * if user sets month to JAN and day to 31st then changes month to feb,
          * that will create an impossible date "31st feb" so change day to 28th
          * (the highest day in that month)
          */
          if (editingRow == 2 || editingRow == 3) // YEAR || MONTH
            adjust_days_for_month();
        }
      }
    }

    // after one ish hours leave editing mode (accidentally left on)
    if (++loopCount == -32768) {
      timeEdited = false;
      break;
    }
  }

  // only update time if time changed
  if (timeEdited) {
    // change rtc time to edited time
    rtc.setTime(clockTime);
    // give delay for rtc time to be changed before going back into sleep mode
    delay(40);
  }

  // unhighlight editing row
  unhighlight_row(editingRow);

  // reset switch
  edit_switch_activated = false;
}

void adjust_days_for_month() {
  byte daysInMonth = getDaysInMonth();
  if (clockTime[DAY] > daysInMonth) {
    clockTime[DAY] = daysInMonth;
    lm.setRow(4, clockTime[DAY]);
  }
}


void edit_clock_row_value(byte row, byte low, byte high, bool increase) {
  byte arrayRow = 7-row;
  // increase/decrease value
  if (increase) { // increase
    if (clockTime[arrayRow] >= high)  // 59
      clockTime[arrayRow] = low;  // 00
    else
      clockTime[arrayRow]++;
  } else {  // decrease
    if (clockTime[arrayRow] == low) // 00
      clockTime[arrayRow] = high; // 59
    else
      clockTime[arrayRow]--;
  }
  // update clock display
  highlight_row(row);
}

/*
* ----x-xx
* changes to
* xx--x-xx
*/
void highlight_row(byte row) {
  lm.setRow(row, clockTime[7-row]|192);
}

void unhighlight_row(byte row) {
  lm.setRow(row, clockTime[7-row]);
}
