#include "encoder.h"
#include <xc.h>

#define MID_TICKS 32768

static int encoder_command(int read) { // send a command to the encoder chip
                                       // 0 = reset count to 32,768, 1 = return the count
  SPI4BUF = read;                      // send the command
  while (!SPI4STATbits.SPIRBF) { ; }   // wait for the response
  SPI4BUF;                             // garbage was transferred, ignore it
  SPI4BUF = 5;                         // write garbage, but the read will have the data
  while (!SPI4STATbits.SPIRBF) { ; }
  return SPI4BUF;
}

void encoder_reset(void) {
    // resets the encoder ticks to 'zero' i.e. the center value
    encoder_command(0);
    encoder_command(0);
}

int encoder_angle(void){
    // read the encoder angle in 1/10 degrees
    int ticks = encoder_ticks();
    int angle = MID_TICKS - ticks;
    return angle << 1; // ticks * (rotation / (448 * 4)) *(3600 10ths of degrees/ rotation) = ticks * 2
}

// change counts to ticks
int encoder_ticks(void) {
  encoder_command(1); // call it twice!
  return encoder_command(1);
}

void encoder_init(void) {
  // SPI initialization for reading from the decoder chip
  SPI4CON = 0;              // stop and reset SPI4
  SPI4BUF;                  // read to clear the rx receive buffer
  SPI4BRG = 0x4;            // bit rate to 8 MHz, SPI4BRG = 80000000/(2*desired)-1
  SPI4STATbits.SPIROV = 0;  // clear the overflow
  SPI4CONbits.MSTEN = 1;    // master mode
  SPI4CONbits.MSSEN = 1;    // slave select enable
  SPI4CONbits.MODE16 = 1;   // 16 bit mode
  SPI4CONbits.MODE32 = 0;
  SPI4CONbits.SMP = 1;      // sample at the end of the clock
  SPI4CONbits.ON = 1;       // turn SPI on
}

// you write functions to reset encoder and return angle in 1/10th degrees
