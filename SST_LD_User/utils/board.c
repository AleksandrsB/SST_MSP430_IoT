/*
 * board.c
 *
 *  Created on: 2024. gada 12. marts
 *      Author: Klavins Eriks
 */

#include "board.h"
#include "Typedefs.h"

extern __Status_t Status;
extern char UART_RxBuf[8];

void board_init()
{
    WDTCTL = WDTPW | WDTHOLD;       // stop watchdog timer
    // SET MAIN DCO to generate 8MHz clock  FOR MCLK & SMCLK
    BCSCTL1 = CALBC1_8MHZ;
    DCOCTL = CALDCO_8MHZ;

    BCSCTL3 &= 0xCF;        // clear LFXT1S settings
    BCSCTL3 |= LFXT1S_2;    // set ACLK to LP/LF = 12kHz

    P1DIR = 0x03; // P1.0 & P1.1 lines set as output
    P1REN = 0x04; // enable Resistor on P1.2 line (PULL-DOWN)
    P1OUT = 0x04; // set P1.2 Resistor as PULL-UP

    P1IFG = 0;
    P1IE = 0x04; // enable interrupt on P1.2 line
    P1IES &= 0xFB; // P1.2 Interrupt Flag is set on Low-to-High transition
}

void delay_ms( unsigned int ms)
{
    if(ms<5461) // MAX milliseconds
    {
        TAR = 0; // clear TIMERA counter register
        TACCR0 = 12*ms; // 12 ACLK cycles for 1 ms
        TACCTL0 = 0x0010;  // enable CC0 interrupt
        TACTL = 0x0110; // TIMERA source =ACLK / counting UP to TACCR0
        __low_power_mode_3();
    }
}

void delay_us( unsigned int us)
{
    if(us<16383)
    {
        // works correctly @ 8MHz SMCLK clock speed  ==> 0,000000125 seconds --> 125 ns = 0,125us
        TAR = 0; // clear TIMERA counter register
        TACCR0 = 4*us; // 4 SMCLK cycles for 1 us    MAX 16383 us
        TACCTL0 = 0x0010;  // enable CC0 interrupt
        TACTL = 0x0210; // TIMERA source =SMCLK / counting UP to TACCR0
        __low_power_mode_0();
    }
}

void wait()
{
    __low_power_mode_0();
}

void SPI_Init()
{
    P3DIR |= 0x0B; //
    P3OUT |= 0x01; // set P3.0 high

    UCB0CTL1 = UCSWRST;
    UCB0CTL1 = UCSWRST | UCSSEL1;
    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC;; // MSB first

    UCB0BR0 = 4;// form 8MHz/4 --> 2MHz      16Mhz/8 --> gives 2MHz spi clock speed
    UCB0BR1 = 0;

    P3SEL |= 0x0E; // P3.7 .. P3.4 GPIO | P3.3 SPI CLK | P3.2 SPI MISO | P3.1 SPI MOSI | P3.0 GPIO (SPI CSN)

    UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

}

void SPI_Send(char data)
{
    IFG2 &= ~UCB0RXIFG;
    UCB0TXBUF = data;
    while(!(IFG2 & UCB0RXIFG));
}

void SPI_Read(char *pData)
{
    *pData = UCB0RXBUF;
}


#pragma vector=PORT1_VECTOR
__interrupt void Port1_ISR()
{
    if(P1IFG & BIT2)    // if(true) {then} eles {}  FALSE=0     true!=FALSE
    {
        P1IFG &= 0xFB; // clear interrupt flag on P1.2
        Status.Button = 1;
    }

    __low_power_mode_off_on_exit(); // Go to ACTIVE MODE
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TIMERA0()
{
    TACTL &= 0xFFCF; // Stop TIMERA
    __low_power_mode_off_on_exit(); // Go to ACTIVE MODE
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC_Done(void)
{
    adcStatus.DataReady = 1;
    adcStatus.ADC_Val = ADC10MEM;
    __low_power_mode_off_on_exit(); // Go to ACTIVE MODE
}

