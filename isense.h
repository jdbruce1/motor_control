#ifndef ISENSE__H__
#define ISENSE__H__

#include "NU32.h"

void isense_init();          // initialize the current sensor module

unsigned int isense_counts();          // read the current sensor, in ADC counts

int isense_curr();          // read the current sensor, in milliamps

#endif
