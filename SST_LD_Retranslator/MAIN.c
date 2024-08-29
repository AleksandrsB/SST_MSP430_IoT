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



    unsigned int i=0;

    unsigned int TD=100; // default 100ms
    unsigned char OwnAddress = 1;
    unsigned char SensorDirAddr = 0xFF;
    unsigned char sensorDist=255;

    unsigned int lastServPing=0;

    CC_SetChannel(100);

    CC_SetAddress(OwnAddress); // OWN Address
    ADC_SetChannel(1);

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
        // each 10*TD milliseconds check if target is alive
        if(i%10==0)
        {
            lastServPing++;
            // if after 10*TD * 3 milliseconds we dont have any info from the target -> delete target and reset info
            if(lastServPing>3)
            {
                SensorDirAddr=255;
                sensorDist=255;
            }

        }
        // broadcast for I AM RETRO (only if we have target)
        if(sensorDist<255 && i%10==0)
        {
            CC_tx.ADDRESS = 255; // bradcast address
            CC_tx.DATA[0] = OwnAddress;
            CC_tx.DATA[1] = 'R';
            CC_tx.DATA[2] = 'E';
            CC_tx.DATA[3] = 'T';
            CC_tx.DATA[4] = sensorDist+1; //send my distance to the server
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
            // RETRANSLATOR is within sensor broadcast radius (we can hear the server)
            if (CC_rx.DATA[1] == 'S' && CC_rx.DATA[2] == 'E' && CC_rx.DATA[3] == 'R' && CC_rx.DATA[4] == 'V')
            {
                lastServPing = 0;
                SensorDirAddr = CC_rx.DATA[0]; // UPDATE CLOSEST SENSOR ADDR
                sensorDist=0; // direct connection
            }
            else if(CC_rx.DATA[1] == 'R' && CC_rx.DATA[2] == 'E' && CC_rx.DATA[3] == 'T') // (we can hear other retranslators)
            {
                if(CC_rx.DATA[0] == SensorDirAddr) // we can hear out target => reset 'check for alive' counter
                {
                    lastServPing = 0;
                }
                if(CC_rx.DATA[4] < sensorDist) // if we found better retranslator, then swith to them
                {
                    SensorDirAddr = CC_rx.DATA[0]; // UPDATE CLOSEST SENSOR ADDR (retranslator)
                    sensorDist= CC_rx.DATA[4] ; // direct connection
                    lastServPing = 0;
                }
            }
            else if (CC_rx.DATA[0]==SensorDirAddr) // retranslate server responce
            {
                CC_tx.ADDRESS = 255;
                CC_tx.DATA[0] = OwnAddress;
                CC_tx.DATA[1] = CC_rx.DATA[1];
                CC_tx.DATA[2] = CC_rx.DATA[2];
                CC_tx.DATA[3] = CC_rx.DATA[3];
                CC_tx.DATA[4] = CC_rx.DATA[4];
                CC_tx.DATA[5] = CC_rx.DATA[5];
                CC_tx.DATA[6] = CC_rx.DATA[6];
                CC_tx.DATA[7] = CC_rx.DATA[7];
                CC_tx.LEN = 8;
            }
            else // retranslate user request
            {
                CC_tx.ADDRESS = SensorDirAddr;
                CC_tx.DATA[0] = OwnAddress;
                CC_tx.DATA[1] = CC_rx.DATA[1];
                CC_tx.DATA[2] = CC_rx.DATA[2];
                CC_tx.DATA[3] = CC_rx.DATA[3];
                CC_tx.DATA[4] = CC_rx.DATA[4];
                CC_tx.DATA[5] = CC_rx.DATA[5];
                CC_tx.DATA[6] = CC_rx.DATA[6];
                CC_tx.DATA[7] = CC_rx.DATA[7];
                CC_tx.DATA[8] = CC_rx.DATA[8];
                CC_tx.LEN = 9;
            }
            P1OUT &= 0xFD;
        }
        i++;
    }
}


