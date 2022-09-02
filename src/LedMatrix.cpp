#include "LedMatrix.h"


LedMatrix::LedMatrix() {
  // enable pins
  pinMode(DIN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(CS, OUTPUT);

  digitalWrite(CS, HIGH);

  spiTransfer(OP_DISPLAYTEST, 0);
  spiTransfer(OP_SCANLIMIT, 7); // allow all 8 rows to be displayed
  spiTransfer(OP_DECODEMODE, 0);

  clear();  // clear display

  shutdown(false); // turn leds on
  spiTransfer(OP_INTENSITY, 2); // intensity
}

// turn off LEDs
void LedMatrix::shutdown(bool state) {
  spiTransfer(OP_SHUTDOWN, !state);
}

void LedMatrix::clear() {
  for (int i=0; i<8; i++)
    spiTransfer(1+i, 0);
}

void LedMatrix::setRow(int row, byte value) {
  row = 7-row;

  if (value != display[row]) {  // check if row is same value before writing
    display[row] = value;
    spiTransfer(row+1, display[row], 0);
  }
}

void LedMatrix::spiTransfer(byte opcode, byte data, bool direction) {
  // enable writing
  digitalWrite(CS,LOW);

  // send data
  shiftOut(DIN, CLK, MSBFIRST, opcode);
  shiftOut(DIN, CLK, direction, data);

  // latch data to display
  digitalWrite(CS,HIGH);
}
