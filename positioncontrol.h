#ifndef POSITIONCONTROL__H__
#define POSITIONCONTROL__H__

#include "NU32.h"
#include "currentcontrol.h"
#include "utilities.h"
#include "encoder.h"

#define MAX_TRAJ 1000 // the maximum trajectory length, corresponds to 5 seconds
#define IMAX 400 // the maximum current control signal (tuneable)

extern volatile float Kpp, Kip, Kdp;

extern int angle_command;

void positioncontrol_init();

extern volatile int mes_pos_array[MAX_TRAJ];  // measured values to plot
extern volatile int ref_pos_array[MAX_TRAJ];  // reference values to plot
extern volatile int traj_len;

// extern volatile int mes_array[100];  // measured values to plot
// extern volatile int ref_array[100];  // reference values to plot

// int encoder_ticks();          // read the encoder, in ticks
//
// int encoder_angle();          // read the encoder angle in 1/10 degrees
//
// void encoder_reset();         // reset the encoder position

#endif
