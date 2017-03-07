#include "NU32.h"          // config bits, constants, funcs for startup and UART
#include "LCD.h"
#include "encoder.h"
#include "currentcontrol.h"
#include "isense.h"
#include "utilities.h"
#include <stdio.h>

#define BUF_SIZE 200

// volatile int PWM_val = 0;
extern int PWM_val;
extern volatile float Kpc, Kic;
extern volatile float Kpp, Kip, Kdp;

extern volatile int mes_array[100];  // measured values to plot
extern volatile int ref_array[100];  // reference values to plot

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
            case 'i':
            {
                // sets position gains
                NU32_ReadUART3(buffer,BUF_SIZE);
                sscanf(buffer, "%f %f %f", &Kpp, &Kip, &Kdp);
                sprintf(lcd_string, "Kp=%1.0f Ki=%1.0f Kd=%1.0f",Kpp, Kip, Kdp);
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
                break;
            }
            case 'j':
            {
                // gets position gains
                sprintf(buffer, "%f %f %f\r\n", Kpp, Kip, Kdp);
                NU32_WriteUART3(buffer);
                sprintf(lcd_string, "Kp=%1.0f Ki=%1.0f Kd=%1.0f",Kpp, Kip, Kdp);
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
                break;
            }
            case 'k':
            {
                // runs current testing
                mode = ITEST;
                while (mode == ITEST){
                    sprintf(lcd_string, "Blocking");
                    LCD_Move(1,0);
                    LCD_WriteString(lcd_string);
                }
                sprintf(buffer, "%d\r\n", 100);
                NU32_WriteUART3(buffer);
                int i;
                for (i=0; i<100; i++){                      // send plot data to MATLAB
                    sprintf(buffer, "%d %d\r\n", mes_array[i], ref_array[i]);
                    NU32_WriteUART3(buffer);
                }
                sprintf(lcd_string, "Current test done");
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
                break;
            }
            case 'h':
            {
                // gets current gains
                sprintf(buffer, "%f %f\r\n", Kpc, Kic);
                NU32_WriteUART3(buffer);
                sprintf(lcd_string, "Kpc=%3.1f Kic=%3.1f ",Kpc, Kic);
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
                break;
            }
            case 'g':
            {
                // sets current gains
                NU32_ReadUART3(buffer,BUF_SIZE);
                sscanf(buffer, "%f %f", &Kpc, &Kic);
                sprintf(lcd_string, "Kpc=%3.1f Kic=%3.1f ",Kpc, Kic);
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
                break;
            }
            case 'f':
            {
                // sets PWM value
                NU32_ReadUART3(buffer,BUF_SIZE);
                sscanf(buffer, "%d", &PWM_val);
                mode = PWM;
                sprintf(lcd_string, "PWM = %d", PWM_val);
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
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
                sprintf(lcd_string, "ADC curr = %d", isense_curr());
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
                break;
            }
            case 'a':
            {
                // reads the ADC counts
                sprintf(buffer, "%d\r\n", isense_counts());
                NU32_WriteUART3(buffer);
                sprintf(lcd_string, "ADC counts = %d", isense_counts());
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
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
                sprintf(lcd_string, "Angle = %d", encoder_angle());
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
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
                sprintf(lcd_string, "Enc cnt = %d", encoder_ticks());
                LCD_Move(1,0);
                LCD_WriteString(lcd_string);
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
