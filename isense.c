#include "isense.h"

#define SAMPLE_TIME 10                  // 10 core timer ticks = 250 ns

unsigned int adc_sample_convert(int pin) {
    // sample & convert the value on the given
    // adc pin. The pin should be configured as an
    // analog input in AD1PCFG

    unsigned int elapsed = 0, finish_time = 0;
    AD1CHSbits.CH0SA = pin;                // connect chosen pin to MUXA for sampling
    AD1CON1bits.SAMP = 1;                  // start sampling
    elapsed = _CP0_GET_COUNT();
    finish_time = elapsed + SAMPLE_TIME;
    while (_CP0_GET_COUNT() < finish_time) {
      ;                                   // sample for more than 250 ns
    }
    while (!AD1CON1bits.DONE) {
      ;                                   // wait for the conversion process to finish
    }
    return ADC1BUF0;                      // read the buffer with the result
}

void isense_init(){
    AD1PCFGbits.PCFG0 = 0;              // AN0 is an adc pin, B0 on the PIC
    AD1CON3bits.ADCS = 2;               // ADC clock period is Tad = 2*(ADCS+1)*Tpb = 2*3*12.5ns = 75ns
    AD1CON1bits.SSRC = 0b111;           // sets manual conversion
    AD1CON1bits.ADON = 1;               // turn on A/D converter
}

unsigned int isense_counts(){
    int val1 = adc_sample_convert(0);     // get analog value from pin AN0
    int val2 = adc_sample_convert(0);
    int val3 = adc_sample_convert(0);
    int val4 = adc_sample_convert(0);
    return (val1+val2+val3+val4)/4;
}

int isense_curr(){
    unsigned int counts = isense_counts();
    unsigned int intermediate = (counts << 2) - (counts >> 1); // 3.5*counts = 4*counts - counts/2
    int milliamps = intermediate - 1760;
    // test code to see if I can get better results
    milliamps = (milliamps - 15) * 3 / 2;
    return milliamps;
}
