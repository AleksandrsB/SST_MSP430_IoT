/*
 * adc.h
 *
 *  Created on: 2024. gada 12. marts
 *      Author: Klavins Eriks
 */

#ifndef ADC_H_
#define ADC_H_

#include <msp430.h>
#include "Typedefs.h"
#include "board.h"
__ADC_Status_t adcStatus;

void ADC_init();
void ADC_SetChannel(unsigned char ch);
inline int ADC_getVal();
void ADC_GetInternalTemp(float *temp);

void ADC_GetR1(float *R1, float R2);
void ADC_GetR2(float *R2, float R1);

void convert_RtoTemp(float R, float* temp);

#endif /* ADC_H_ */
