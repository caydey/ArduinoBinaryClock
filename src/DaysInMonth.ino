const byte daysInMonth[12] PROGMEM = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

byte getDaysInMonth() {
  if (clockTime[MONTH] == 2) // february
    if ((clockTime[YEAR] & 3) == 0) // leap year (& 3 = % 4)
      return 29;
  return pgm_read_byte(&daysInMonth[clockTime[MONTH]-1]);
}