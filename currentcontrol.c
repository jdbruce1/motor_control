// #include "currentcontrol.h"
//
// volatile int PWM_val = 0;
//
// void __ISR(_TIMER_2_VECTOR, IPL5SOFT) current_controller(void){
//     // interrupt for 5kHz current controller
//     // sets the next value for PWM on cycle values
//     OC1RS = 1000;                       // duty cycle = OC1RS/(PR3+1) = 25%
//
//     // LATDINV = 0x2;                      // invert pin D1
//
//     int PWM_mag;
//     int PWM_dir;
//
//     switch (get_mode()) {
//         case 1: //IDLE
//             // put H-bridge in brake mode
//             // turn off PWM
//             OC1RS = 0; // I think this is the best way to do this.
//             break;
//         case 2: // PWM
//             // set PWM value according to user specified value
//
//             if (PWM_val >= 0){
//                 PWM_dir = 1;
//                 PWM_mag = PWM_val;
//             } else{
//                 PWM_dir = 0;
//                 PWM_mag = -PWM_val;
//             }
//
//             OC1RS = PWM_mag * 40;       // set pwm duty cycle by magnitude of PWM, scaled to 1 period = 4000 counts
//
//             LATDbits.LATD1 = PWM_dir;   // set phase by sign of PWM_val
//
//             break;
//         case 3: // ITEST
//             break;
//         case 4: // HOLD
//             break;
//         case 5: // TRACK
//             break;
//     }
//
//
//
//     IFS0bits.T2IF = 0;                  // clear interrupt flag
//
// }
//
// void currentcontrol_init(){
//     // timer and interrupt for 5kHz ISR
//     // uses timer 2
//
//     T2CONbits.TCKPS = 0;                // Timer2 prescaler N=1
//     PR2 = 15999;                        // period = (PR2+1) * N * 12.5 ns = 200 us, 5 kHz
//     TMR2 = 0;                           // initial TMR2 count is 0
//     T2CONbits.TGATE = 0;                //             not gated input (the default)
//     T2CONbits.ON = 1;                   // turn on Timer2
//
//     IPC2bits.T2IP = 5;                  // step 4: interrupt priority 5
//     IPC2bits.T2IS = 0;                  // step 4: interrupt subpriority 0
//     IFS0bits.T2IF = 0;                  // step 5: clear the int flag
//     IEC0bits.T2IE = 1;                  // step 6: enable INT0 by setting IEC0<3>
//
//
//     // timer and OC for 20kHz PWM
//     // Set up PWM on OC1 / Timer3
//     // OC1 is on pin D0
//     T3CONbits.TCKPS = 0;                // Timer3 prescaler N=1 (1:1)
//     PR3 = 3999;                         // period = (PR3+1) * N * 12.5 ns = 50 us, 20 kHz
//     TMR3 = 0;                           // initial TMR3 count is 0
//     OC1CONbits.OCTSEL = 1;              // select Timer 3
//     OC1CONbits.OCM = 0b110;             // PWM mode without fault pin; other OC1CON bits are defaults
//     OC1RS = 1000;                       // duty cycle = OC1RS/(PR3+1) = 25%
//     OC1R = 1000;                        // initialize before turning OC1 on; afterward it is read-only
//     T3CONbits.ON = 1;                   // turn on Timer3
//     OC1CONbits.ON = 1;                  // turn on OC1
//
//
//         // digital output to control motor direction
//         // use RD1, it's right next to D0 for the PWM
//
//     TRISDbits.TRISD1 = 0;               // set D1 as digital output
//     LATDbits.LATD1 = 1;                 // set D1 high to start
// }
