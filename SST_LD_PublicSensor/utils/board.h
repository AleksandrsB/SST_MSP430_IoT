/*
 * board.h
 *
 *  Created on: 2024. gada 12. marts
 *      Author: Klavins Eriks
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <msp430.h>
#include <intrinsics.h>
#include "adc.h"

void board_init();

void delay_ms( unsigned int ms);
void delay_us( unsigned int us);
void wait();

void SPI_Init();
void SPI_Send(char data);
void SPI_Read(char *pData);


#endif /* BOARD_H_ */
