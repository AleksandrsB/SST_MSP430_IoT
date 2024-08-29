/*
 * MAIN.c
 *
 *  Created on: 2024. gada 7. febr.
 *      Author: Klavins Eriks, Buraks Aleksandrs
 *      USER
 */
#include <msp430.h>         // F 2274
#include <intrinsics.h>

#include "MAIN.h"

unsigned int seed = 1234; // Initial seed value
const unsigned int multiplier = 11035;
unsigned int increment = 12345;

unsigned char random_byte() {
    seed = (seed * multiplier + increment) & 0xFFFF;
    return seed & 0xFF;
}

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

    unsigned int TD=100; // default 100ms
    unsigned char OwnAddress = 1;
    unsigned char SensorDirAddr = 0xFF;
    unsigned char sensorDist=255;

    CC_SetChannel(100);
    CC_SetAddress(OwnAddress); // OWN Address

    ADC_SetChannel(1);

    unsigned int i=0;

    char buttonClicked=0;
    unsigned int iterCounter=0;
    unsigned char key; // key for encription

    while(1) // ######################### MAIN LOOP #########################
    {
        delay_ms(TD);
        // if button clicked first time => start "timer" (iterCounter) with length = 10*TD milliseconds
        // within 10*TD milliseconds all new button clicks will be counted in buttonClicked++
        if(buttonClicked!=0)
        {
            // we need this to count button clicks within TD*10 milliseconts
            iterCounter++;
        }
        // on "timer" end
        if(iterCounter>10)
        {
            // if button clicked 1 time within TD*10 ms
            if(buttonClicked==1)
            {
                // send temperature request
                CC_tx.ADDRESS = SensorDirAddr;
                key = random_byte(); // for encription
                CC_tx.DATA[0] = OwnAddress;
                CC_tx.DATA[1] = 'T';
                CC_tx.DATA[2] = 'E';
                CC_tx.DATA[3] = 'M';
                CC_tx.DATA[4] = 'P';
                CC_tx.DATA[5] = key; // send encryption key to server
                CC_tx.LEN = 6;
                CC_SendMessage(CC_tx);
            }
            else // multiple click
            {
                // change TD on PublicSensor (100ms -> 200ms)
                CC_tx.ADDRESS = SensorDirAddr;
                CC_tx.DATA[0] = OwnAddress;
                CC_tx.DATA[1] = 'C';
                CC_tx.DATA[2] = 'H';
                CC_tx.DATA[3] = 'G';
                CC_tx.DATA[4] = 200;
                CC_tx.LEN = 5;
                CC_SendMessage(CC_tx);
            }
            // reset "timer" and button clicks
            buttonClicked=0;
            iterCounter=0;
        }
        if(Status.Button==1) // catch button single click
        {

            buttonClicked++;
            delay_ms(100);
            Status.Button=0;
        }

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

        // USER

        CC_SetWaitState(CC2500_SRX, cc_RX);
        CC_CheckERROR();

        P1OUT ^= 0x01;

        // RESPONCE  RECEIVED ===== USER
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

            // if we can hear server => than user is within sensor broadcast radius
            if (CC_rx.DATA[1] == 'S' && CC_rx.DATA[2] == 'E' && CC_rx.DATA[3] == 'R' && CC_rx.DATA[4] == 'V')
            {
                SensorDirAddr = CC_rx.DATA[0]; // UPDATE CLOSEST SENSOR ADDR
                sensorDist=0; // direct connection
            }
            else if(CC_rx.DATA[1] == 'R' && CC_rx.DATA[2] == 'E' && CC_rx.DATA[3] == 'T' && CC_rx.DATA[4] < sensorDist) // we hear retranslator
            {
                SensorDirAddr = CC_rx.DATA[0]; // UPDATE CLOSEST SENSOR ADDR (retranslator)
                sensorDist= CC_rx.DATA[4] ; // direct connection
            }
            else if (CC_rx.DATA[0]==SensorDirAddr && CC_rx.DATA[1] == 'T' && CC_rx.DATA[2] == 'R' && CC_rx.DATA[3] == 'P') // user receive responce
            {
                __TypeCast r;
                r.b[0]= CC_rx.DATA[4];
                r.b[1]= CC_rx.DATA[5];
                r.b[2]= CC_rx.DATA[6];
                r.b[3]= CC_rx.DATA[7];
                // restore the value using saved key (saved on pre-request)
                r.b[0] = r.b[0]^key;
                r.b[1] = r.b[1]^key;
                r.b[2] = r.b[2]^key;
                r.b[3] = r.b[3]^key;
                float resultTemp = r.f;
            }
            P1OUT &= 0xFD;
        }
        i++;
    }
}


