#ifndef POSITIONCONTROL__H__
#define POSITIONCONTROL__H__

#include "NU32.h"

extern volatile float Kpp, Kip, Kdp;

void positioncontrol_init();

// extern volatile int mes_array[100];  // measured values to plot
// extern volatile int ref_array[100];  // reference values to plot

// int encoder_ticks();          // read the encoder, in ticks
//
// int encoder_angle();          // read the encoder angle in 1/10 degrees
//
// void encoder_reset();         // reset the encoder position

#endif
