// modified library from https://github.com/wayoda/LedControl
#ifndef LedMatrix_h
#define LedMatrix_h

#include <Arduino.h>

#define DIN 12
#define CLK 10
#define CS  11

#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

class LedMatrix {
  private:
    byte display[8];
    void spiTransfer(byte opcode, byte data, bool direction=1);
  public:
    LedMatrix();

    void shutdown(bool status);
    void clear();
    void setRow(int row, byte value);
};

#endif
