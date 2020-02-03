#include "msp.h"

/**
 * main.c
 */

void waitTime(unsigned int time){
    unsigned int i = 0;
    while(i < time){
        i++;
    }
}

void display_numbers(){
    while(1){
        //Number 1
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x06; //Adjusts what turns on.
        P8->OUT &= ~BIT5;
        waitTime(10);

        //Number 2
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x5B;
        P8->OUT &= ~BIT4;
        waitTime(10);

        //Number 3
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x4f;
        P8->OUT &= ~BIT3;
        waitTime(10);

        //Number 4
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x66;
        P8->OUT &= ~BIT2;
        waitTime(10);
    }
}

void display_key(char key) {
    unsigned int displays[] = {BIT5, BIT4, BIT3, BIT2};
    //unsigned int i = 0;
    switch(key) {
        case '1':
            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
            P4->OUT = ~0x06;
            //P8->OUT &= ~displays[i];
            break;
        case '2' :
            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
            P4->OUT = ~0x5B;
            P8->OUT &= ~displays[1];
            break;
        case '3' :
            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
            P4->OUT = ~0x4F;
            P8->OUT &= ~displays[2];
            break;
        case 'A' :
            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
            P4->OUT = 0x88;
            P8->OUT &= ~displays[3];
            break;
    }
}

void scan(){
    unsigned int k = 0;
    unsigned int cols[] = {BIT3, BIT2, BIT1, BIT0};

    P8->DIR |= BIT5;
    P9->DIR &= ~BIT3;
    P8->OUT &= ~BIT5;
    P9->OUT &= ~BIT3;

    while(1){
        if(P9->IN & cols[k]) {
            switch(k) {
                case(0): display_key('1');
                case(1): display_key('2');
                case(2): display_key('3');
                case(3): display_key('A');
            }
        }
        k = k + 1;
        if(k == 4){
            k = 0;
        }
    }
}

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    P8->DIR = 0xff;
    P4->DIR = 0xff;

    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = 0xC0; //Adjusts what turns on.
    P8->OUT &= ~BIT5;

    scan();

}





