/*
 * adc.c
 *
 *  Created on: 2024. gada 12. marts
 *      Author: Klavins Eriks
 *
 */

#include "adc.h"
#include <math.h>

void ADC_init()
{
    ADC10CTL0 = ADC10ON | ADC10IE;
    ADC10CTL1 = 0xB000 | ADC10SSEL_3; // CLK source = SMCLK  input channel = B
    ADC10CTL0 |= 0x0002;  // set ENC bit to enable conversion
}

void ADC_SetChannel(unsigned char ch)
{
    if(adcStatus.channel == ch)
        return;
    // check for ch variable  0..15  -> [0 1 2 3 4   10 11 12 13 14 15]
    if((ch>4 && ch<10) || ch>15 ) // deny ch 5..9 and all greater than 15
        return;

    ADC10CTL0 &= 0xFFFD; // set ENC bit to 0 --> Disable conversion
    if(ch==10) // check for inner channels (ch=10 temperature sensor)
    {
        ADC10CTL0 &= 0x1FFF; // Clear ADC reference voltage settings --> Vref = AVcc
        ADC10CTL0 |= 0x2000; // set ADC Reference voltage to internal 1.5V
        ADC10CTL1 &= 0x0FFF; // clear input channel settings --> ADC in = IN0
        ADC10CTL1 |= 0xA000; // Set input channel = 10 (0Ah) => internal temperature sensor
    }
    else
    {
        ADC10CTL0 &= 0x1FFF; // Clear ADC reference voltage settings --> Vref = AVcc
        ADC10CTL1 &= 0x0FFF; // clear input channel settings --> ADC in = IN0
        ADC10CTL1 |= (ch << 12) & 0xF000; // Set input channel
    }
    adcStatus.channel = ch;
    ADC10CTL0 |= 0x0002;  // set ENC bit to 1 --> enable conversion
}

inline int ADC_getVal()
{
    adcStatus.DataReady = 0;
    ADC10CTL0 |= ADC10SC | ADC10IE; // start conversion
    while(!adcStatus.DataReady)
        wait();
    return ADC10MEM;
}

void ADC_GetInternalTemp(float *temp)
{
    ADC_SetChannel(10); // read temperature sensor
    adcStatus.DataReady = 0;
    ADC10CTL0 |= ADC10SC; // Start conversion
    while(!adcStatus.DataReady)
        wait();

    int raw = ADC10MEM;

/*    *temp = ((float)raw)*0.0014648;
    *temp = *temp - 0.986;
    *temp = *temp/0.00355;*/

    *temp = (raw - 673)*412;
    *temp = *temp/1024;
}

void ADC_GetR1(float *R1, float R2)
{
    float tmp = R2 * 1024.0;
    int adc = ADC_getVal();

    *R1 = tmp/adc;
    *R1 -= R2;
}

void ADC_GetR2(float *R2, float R1)
{
    int adc = ADC_getVal();
    float tmp = adc * 3;

    *R2 = tmp*R1;
    tmp = 3072-tmp;
    *R2 = *R2/tmp;
}

void convert_RtoTemp(float R, float* temp)
{
    float tmp = log(R); // ln(R)
    //tmp = tmp/M_LOG10E;
    float tmp2 = tmp*tmp; // ^2
    float tmp3 = tmp2*tmp; // ^3
    float _B = 0.00022826240571 * tmp; //-
    float _C = 0.00000052152023103519 * tmp2;//-
    float _D = 0.00000007484370007126 * tmp3;//-

    tmp=(0.001148925920362 + _B);
    tmp = tmp + _C;
    tmp = tmp + _D;

    *temp = 1.0/tmp;
}
