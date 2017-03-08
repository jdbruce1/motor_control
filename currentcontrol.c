#include "currentcontrol.h"

int PWM_val = 0; // it doesn't have to be volatile; it's only read in the ISR

volatile float Kpc = 0, Kic = 0;   // control gains for current
static volatile int itest_counter = 0;
static volatile int i_ref = 200;
static volatile int Eint = 0;

volatile int current_command; // sent from position controller

volatile int mes_array[100];  // measured values to plot
volatile int ref_array[100];  // reference values to plot

void set_PWM_from_val(int PWM_setting){

    int PWM_mag;
    int PWM_dir;

    if (PWM_setting >= 0){
        PWM_dir = 1;
        PWM_mag = PWM_setting;
    } else{
        PWM_dir = 0;
        PWM_mag = -PWM_setting;
    }

    OC1RS = PWM_mag * 40;       // set pwm duty cycle by magnitude of PWM, scaled to 1 period = 4000 counts

    LATDbits.LATD1 = PWM_dir;   // set phase by sign of PWM_val

}

int pi_current_controller(int measured, int reference){
    // uses set gains to provide control
    // takes in the measured and reference ADC counts
    // outputs the OC1RS setting to reach reference values

    int error;                          // the absolute error in ADC counts
    float u;                            // the control effort in floating point
    int u_norm;                         // the control effort, normalized (first to percentage, then to OC1RS setting)

    error = reference - measured;
    Eint += error;
    u = (Kpc * error) + (Kic * Eint);     // u calculated in floating point

    if (u > 100){  // u_norm is the PWM_val
        u_norm = 100;
    } else if (u < -100){
        u_norm  = -100;
    } else{
        u_norm = (int) u;
    }

    // u_norm = (u_norm * PR3) / 100;

    return u_norm;
}

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) current_controller(void){
    // interrupt for 5kHz current controller
    // sets the next value for PWM on cycle values
    // OC1RS = 1000;                       // duty cycle = OC1RS/(PR3+1) = 25%

    int control_effort;

    // LATDINV = 0x2;                      // invert pin D1

    switch (get_mode()) {
        case 1: //IDLE
            // put H-bridge in brake mode
            // turn off PWM
            OC1RS = 0; // I think this is the best way to do this.
            break;
        case 2: // PWM
            // set PWM value according to user specified value

            set_PWM_from_val(PWM_val);
            break;
        case 3: // ITEST
            // generate reference current or loop measurement
            if (itest_counter < 25) i_ref = 200;
            else if (itest_counter == 25) i_ref = -200;
            else if (itest_counter == 50) i_ref = 200;
            else if (itest_counter == 75) i_ref = -200;
            else if (itest_counter == 99){
                itest_counter = 0;
                enum mode_t new_mode = IDLE;
                set_mode(new_mode);
            }

            // save values for plotting

            mes_array[itest_counter] = isense_curr();
            ref_array[itest_counter] = i_ref;

            // calculate effort and set PWM
            control_effort = pi_current_controller(mes_array[itest_counter], i_ref);
            // mes_array[itest_counter] = control_effort; //temporary, view control effort output
            set_PWM_from_val(control_effort);

            itest_counter++;

            break;
        case 4: // HOLD
            // use the current command, as set by position controller
            LATDINV = 0b1000;       // invert pin D3
            control_effort = pi_current_controller(isense_curr(), current_command);
            set_PWM_from_val(control_effort);
            break;
        case 5: // TRACK
            break;
    }

    IFS0bits.T2IF = 0;                  // clear interrupt flag

}

void currentcontrol_init(){
    // timer and interrupt for 5kHz ISR
    // uses timer 2

    T2CONbits.TCKPS = 0;                // Timer2 prescaler N=1
    PR2 = 15999;                        // period = (PR2+1) * N * 12.5 ns = 200 us, 5 kHz
    TMR2 = 0;                           // initial TMR2 count is 0
    T2CONbits.TGATE = 0;                //             not gated input (the default)
    T2CONbits.ON = 1;                   // turn on Timer2

    IPC2bits.T2IP = 5;                  // step 4: interrupt priority 5
    IPC2bits.T2IS = 0;                  // step 4: interrupt subpriority 0
    IFS0bits.T2IF = 0;                  // step 5: clear the int flag
    IEC0bits.T2IE = 1;                  // step 6: enable INT0 by setting IEC0<3>


    // timer and OC for 20kHz PWM
    // Set up PWM on OC1 / Timer3
    // OC1 is on pin D0
    T3CONbits.TCKPS = 0;                // Timer3 prescaler N=1 (1:1)
    PR3 = 3999;                         // period = (PR3+1) * N * 12.5 ns = 50 us, 20 kHz
    TMR3 = 0;                           // initial TMR3 count is 0
    OC1CONbits.OCTSEL = 1;              // select Timer 3
    OC1CONbits.OCM = 0b110;             // PWM mode without fault pin; other OC1CON bits are defaults
    OC1RS = 1000;                       // duty cycle = OC1RS/(PR3+1) = 25%
    OC1R = 1000;                        // initialize before turning OC1 on; afterward it is read-only
    T3CONbits.ON = 1;                   // turn on Timer3
    OC1CONbits.ON = 1;                  // turn on OC1


    // digital output to control motor direction
    // use RD1, it's right next to D0 for the PWM

    TRISDbits.TRISD1 = 0;               // set D1 as digital output
    LATDbits.LATD1 = 1;                 // set D1 high to start

    TRISDbits.TRISD3 = 0;               // set D3 as digital output
    LATDbits.LATD3 = 1;                 // set D3 high to start

}
