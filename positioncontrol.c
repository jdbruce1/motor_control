#include "positioncontrol.h"
#include "currentcontrol.h"

volatile float Kpp = 0, Kip = 0, Kdp = 0; // position control gains

void __ISR(_TIMER_4_VECTOR, IPL4SOFT) position_controller(void){

    LATDINV = 0b100;    // toggle digital output

    IFS0bits.T4IF = 0;                  // clear interrupt flag
}

void positioncontrol_init(){

    // setup for 200 Hz ISR on Timer4
    // we're going to need a prescaler here, probably not that large though

    // \/ all this is wrong, I just copied it from the other function

    T4CONbits.TCKPS = 0b11;             // Timer4 prescaler N=8
    PR4 = 49999;                        // period = (PR4+1) * N * 12.5 ns = 5 ms, 200 Hz
    TMR4 = 0;                           // initial TMR2 count is 0
    T4CONbits.TGATE = 0;                //             not gated input (the default)
    T4CONbits.ON = 1;                   // turn on Timer2

    IPC4bits.T4IP = 4;                  // step 4: interrupt priority 4
    IPC4bits.T4IS = 0;                  // step 4: interrupt subpriority 0
    IFS0bits.T4IF = 0;                  // step 5: clear the int flag
    IEC0bits.T4IE = 1;                  // step 6: enable INT0 by setting IEC0<3>

    TRISDbits.TRISD2 = 0;               // set D2 as digital output
    LATDbits.LATD2 = 1;                 // set D2 high to start
}
