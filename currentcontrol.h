#ifndef CURRENTCONTROL__H__
#define CURRENTCONTROL__H__

#include "NU32.h"

extern int PWM_val;

extern volatile float Kpc, Kic;   // control gains for current

extern volatile int mes_array[100];  // measured values to plot
extern volatile int ref_array[100];  // reference values to plot

void currentcontrol_init();          // initialize the current control module

// int encoder_ticks();          // read the encoder, in ticks
//
// int encoder_angle();          // read the encoder angle in 1/10 degrees
//
// void encoder_reset();         // reset the encoder position

#endif
