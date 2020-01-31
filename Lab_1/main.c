#include "msp.h"


/**
 * main.c
 */
void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    P8->DIR = 0xff;
    P4->DIR = 0xff;
    P8->OUT |= (BIT4 | BIT3| BIT2);
    P8->OUT &= ~BIT5;
    P4->OUT = ~0x01; //Adjusts what turns on.
}
