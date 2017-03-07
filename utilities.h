#ifndef UTILITIES__H__
#define UTILITIES__H__

#include "NU32.h"

enum mode_t {IDLE, PWM, ITEST, HOLD, TRACK};

enum mode_t mode;

void set_mode(enum mode_t);

int get_mode(void);

#endif
