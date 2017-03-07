#include "utilities.h"

enum mode_t mode = IDLE;

void set_mode(enum mode_t new_mode){
    // sets the mode by a mode enum
    mode = new_mode;
}

int get_mode(){
    // gets the mode as an integer
    // this should match up with the MATLAB code
    switch (mode) {
        case IDLE:
            return 1;
        case PWM:
            return 2;
        case ITEST:
            return 3;
        case HOLD:
            return 4;
        case TRACK:
            return 5;
        default:
            return 0;
    }
}
