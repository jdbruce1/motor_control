#include "NU32.h"          // config bits, constants, funcs for startup and UART
#include "LCD.h"
#include "encoder.h"
// #include "currentcontrol.h"
#include "isense.h"
#include <stdio.h>

#define BUF_SIZE 200

// volatile int PWM_val = 0;

// this should maybe be in a utilities.h file
enum mode_t {IDLE, PWM, ITEST, HOLD, TRACK};
enum mode_t mode = IDLE;

void set_mode(enum mode_t new_mode){
    // sets the mode by a mode enum
    mode = new_mode;
}

// this stuff belongs in currentcontrol.c
volatile int PWM_val = 0;

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) current_controller(void){
    // interrupt for 5kHz current controller
    // sets the next value for PWM on cycle values
    OC1RS = 1000;                       // duty cycle = OC1RS/(PR3+1) = 25%

    // LATDINV = 0x2;                      // invert pin D1

    int PWM_mag;
    int PWM_dir;

    switch (get_mode()) {
        case 1: //IDLE
            // put H-bridge in brake mode
            // turn off PWM
            OC1RS = 0; // I think this is the best way to do this.
            break;
        case 2: // PWM
            // set PWM value according to user specified value

            if (PWM_val >= 0){
                PWM_dir = 1;
                PWM_mag = PWM_val;
            } else{
                PWM_dir = 0;
                PWM_mag = -PWM_val;
            }

            OC1RS = PWM_mag * 40;       // set pwm duty cycle by magnitude of PWM, scaled to 1 period = 4000 counts

            LATDbits.LATD1 = PWM_dir;   // set phase by sign of PWM_val

            break;
        case 3: // ITEST
            break;
        case 4: // HOLD
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
}
// back to being fine

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

int main(){
    char buffer[BUF_SIZE];
    NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
    NU32_LED1 = 1;  // turn off the LEDs
    NU32_LED2 = 1;
    LCD_Setup();

    __builtin_disable_interrupts();
    // in future, initialize modules or peripherals here
    encoder_init();

    mode = IDLE;
    isense_init();                  // initialize current sensor
    currentcontrol_init();
    PWM_val = 0;                    // remove this later
    __builtin_enable_interrupts();

    // LCD stuff

    char lcd_string[16];
    LCD_Clear();
    LCD_Move(0,0);

    sprintf(lcd_string, "Program Started");
    LCD_WriteString(lcd_string);

    // End LCD test stuff

    while(1)
    {
        NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
        NU32_LED2 = 1;                   // clear the error LED

        // write the command recieved to the LCD
        LCD_Clear();
        LCD_Move(0,0);
        sprintf(lcd_string, "Received %c",buffer[0]);
        LCD_WriteString(lcd_string);
        switch (buffer[0]) {
            case 'f':
            {
                // sets PWM value
                NU32_ReadUART3(buffer,BUF_SIZE);
                sscanf(buffer, "%d", &PWM_val);
                mode = PWM;
                break;
            }
            case 'p':
                mode = IDLE;
                break;
            case 'b':
            {
                // reads the ADC counts
                sprintf(buffer, "%d\r\n", isense_curr());
                NU32_WriteUART3(buffer);
                break;
            }
            case 'a':
            {
                // reads the ADC counts
                sprintf(buffer, "%d\r\n", isense_counts());
                NU32_WriteUART3(buffer);
                break;
            }
            case 'r':
            {
                // reads the encoder mode
                sprintf(buffer, "%d\r\n", get_mode());
                NU32_WriteUART3(buffer);
                break;
            }
            case 'd':
            {
                // reads encoder degrees
                // returns encoder angle in 1/10th degrees to the user
                sprintf(buffer,"%d\r\n", encoder_angle());
                NU32_WriteUART3(buffer);    // send encoder count to client
                break;
            }
            case 'e':
            {
                // resets encoder counts
                // doesn't send anything back
                encoder_reset();
                break;
            }
            case 'c':
            {
                // returns encoder counts currently
                sprintf(buffer,"%d\r\n", encoder_ticks());
                NU32_WriteUART3(buffer);    // send encoder count to client
                break;
            }
            case 'x':
            {
                int a = 0;
                int b = 0;
                NU32_ReadUART3(buffer, BUF_SIZE);
                sscanf(buffer, "%d %d", &a, &b);
                sprintf(buffer,"%d\r\n",a+b);
                NU32_WriteUART3(buffer);
                break;
            }
            // case 'd':                      // dummy command for demonstration purposes
            // {
            //     int n = 0;
            //     NU32_ReadUART3(buffer,BUF_SIZE);
            //     sscanf(buffer, "%d", &n);
            //     sprintf(buffer,"%d\r\n", n + 1); // return the number + 1
            //     NU32_WriteUART3(buffer);
            //     break;
            // }
            case 'q':
            {
                // handle q for quit. Later you may want to return to IDLE mode here.
                mode = IDLE;
                break;
            }
            default:
            {
                NU32_LED2 = 0;  // turn on LED2 to indicate an error
                break;
            }
        }
    }
    return 0;
}
