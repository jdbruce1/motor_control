#include "positioncontrol.h"

int angle_command; // 10*angle

volatile float Kpp = 0, Kip = 0, Kdp = 0; // position control gains

static volatile int Eint = 0;
static volatile int e_prev = 0;

extern volatile int current_command; // sent from position controller

volatile int mes_pos_array[MAX_TRAJ];  // measured values to plot
volatile int ref_pos_array[MAX_TRAJ];  // reference values to plot

static volatile int traj_ind = 0;  // index into trajectory arrays
volatile int traj_len;

int pid_position_controller(int measured, int reference){
    // uses set gains to provide control
    // takes in the measured and reference ADC counts
    // outputs the OC1RS setting to reach reference values

    int error;                          // the absolute error in ADC counts
    float u;                            // the control effort in floating point
    int u_norm;                         // the control effort, normalized (first to percentage, then to OC1RS setting)
    int e_dot;

    error = reference - measured;
    Eint += error;
    e_dot = error - e_prev;
    u = (Kpp * error) + (Kip * Eint) + (Kdp * e_dot);     // u calculated in floating point

    e_prev = error;

    if (u > IMAX){  // u_norm is the current command
        u_norm = IMAX;
    } else if (u < -IMAX){
        u_norm  = -IMAX;
    } else{
        u_norm = (int) u;
    }

    return u_norm;
}


void __ISR(_TIMER_4_VECTOR, IPL4SOFT) position_controller(void){

    switch (get_mode()) {
        case 4: // HOLD
            // set current command
            LATDINV = 0b0100;       // invert pin D2
            current_command = pid_position_controller(encoder_angle(), angle_command);
            break;
        case 5: // TRACK
            if (traj_ind > traj_len){
                traj_ind = 0;
                mode = HOLD;
                break;
            }
            mes_pos_array[traj_ind] = encoder_angle();
            current_command = pid_position_controller(mes_pos_array[traj_ind], ref_pos_array[traj_ind]);

            traj_ind++;
            break;
        default:
            break;
    }



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
