/*
 * MAIN.h
 *
 *  Created on: 2024. gada 12. marts
 *      Author: Klavins Eriks
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "Communication/cc2500.h"
#include "utils/board.h"
#include "utils/adc.h"
#include "utils/Typedefs.h"

char UART_RxBuf[8];
char UART_TxBuf[9];

__Status_t Status;


#endif /* MAIN_H_ */
