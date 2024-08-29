/*
 * MAIN.c
 *
 *  Created on: 2024. gada 7. febr.
 *      Author: Klavins Eriks, Buraks Aleksandrs
 */
#include <msp430.h>         // F 2274
#include <intrinsics.h>

#include "MAIN.h"

void main()
{
    board_init();

    ADC_init();
    SPI_Init(); // Check for MCLK freq --> SPI 2MHz
    CC_Init();

    CC_Protocol_t CC_tx, CC_rx;
    for(int i=0; i<62; i++)
    {
        CC_tx.DATA[i] = 0;
        CC_rx.DATA[i] = 0;
    }

    __bis_SR_register(GIE);

    float RT = 0.0;
    float Temp = 0.0;

    float E_est = 10.0; // set initial error in the estimate
    __TypeCast x;
    x.f = 20.0; // set initial state
    float tmp = 0.0;
    float KG;

    CC_SetChannel(100);

    unsigned int TD=100; // default 100ms
    unsigned char OwnAddress = 55;
    unsigned char GW = 0xFF;

    CC_SetAddress(OwnAddress); // OWN Address
    ADC_SetChannel(1);

    unsigned int i=0;
    unsigned char kf;

    while(1) // ######################### MAIN LOOP #########################
    {
        delay_ms(TD);

        {// check for received messages
            char RxBytes=0;
            CC_ReadReg(CC2500_RXBYTES   | 0xC0, &RxBytes);

            if(RxBytes!=0 && !(RxBytes&0x80)) // the message has been received
            {
                Status.GDO_0_Set = 1;
            }
            else if(RxBytes&0x80)
            {
                CC_Strobe(CC2500_SFRX); // flush RX FIFO
            }
        }


        if(Status.KalmanReady)
            kf=1;
        else
            kf=10;

        do{
            ADC_GetR1(&RT, 10000.0);
            convert_RtoTemp(RT, &Temp);
            Temp = Temp - 273.15; // convert Kelvins to Degrees per Celsius
            /* 1. Calculate Kalman Gain */
            KG = E_est/(E_est + 0.25); // 0.25 is obtained through sensor testing

            /* 2. Estimate the new state of the system */
            tmp = Temp - x.f; // calculate the difference between measurement and estimated state
            x.f = x.f + KG*tmp;

            /* 3. Calculate new error in the estimate */
            E_est = (1-KG)*E_est;
            kf--;
        }while(kf);
        // public sensor

        // broadcast for I AM SERVER
        if(i%10==0)
        {
            CC_tx.ADDRESS = 255; // bradcast address
            CC_tx.DATA[0] = OwnAddress;
            CC_tx.DATA[1] = 'S';
            CC_tx.DATA[2] = 'E';
            CC_tx.DATA[3] = 'R';
            CC_tx.DATA[4] = 'V';
            CC_tx.LEN = 5;

            CC_SendMessage(CC_tx);
        }

        CC_SetWaitState(CC2500_SRX, cc_RX);
        CC_CheckERROR();

        P1OUT ^= 0x01;

        if(Status.GDO_0_Set)
        {
            P1OUT |=0x02;
            Status.GDO_0_Set = 0;
            CC_ReadMessage(&CC_rx);
            if(CC_rx.LEN == 0 || CC_rx.LEN>62)
            {
                P1OUT &= 0xFD;
                continue;
            }
            if (CC_rx.DATA[1] == 'T' && CC_rx.DATA[2] == 'E' && CC_rx.DATA[3] == 'M' && CC_rx.DATA[4] == 'P')
            {
                unsigned char key = CC_rx.DATA[5];
                x.b[0] = x.b[0]^key;
                x.b[1] = x.b[1]^key;
                x.b[2] = x.b[2]^key;
                x.b[3] = x.b[3]^key;

                CC_tx.ADDRESS = CC_rx.DATA[0]; // requestor address
                CC_tx.DATA[0] = OwnAddress;
                CC_tx.DATA[1] = 'T';
                CC_tx.DATA[2] = 'R';
                CC_tx.DATA[3] = 'P';
                CC_tx.DATA[4] = x.b[0];
                CC_tx.DATA[5] = x.b[1];
                CC_tx.DATA[6] = x.b[2];
                CC_tx.DATA[7] = x.b[3];
                CC_tx.LEN = 8;
            }
            else if (CC_rx.DATA[1] == 'C' && CC_rx.DATA[2] == 'H' && CC_rx.DATA[3] == 'G')
            {
                TD =  CC_rx.DATA[4];
            }
            P1OUT &= 0xFD;
        }
        i++;
    }
}


