#include "msp.h"

/**
 * main.c
 */

unsigned int d = 0;
unsigned int row = 0;
unsigned int col = 0;
unsigned int nums[] = {0x00, 0x00, 0x00, 0x00};
unsigned int displays[] = {BIT5, BIT4, BIT3, BIT2};
unsigned int rows[] = {BIT5, BIT4, BIT3, BIT2};
unsigned int cols[] = {BIT3, BIT2, BIT1, BIT0};
unsigned int keys[4][4] = {
                         {0x06, 0x5B, 0x4F, 0x77},
                         {0x66, 0x6D, 0x7D, 0x7C},
                         {0x07, 0x7F, 0x67, 0x39},
                         {0x00, 0x3F, 0x00, 0x5E}
};

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

void release_debouncing() {
    int count = 0;
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P8->DIR |= rows[row];
    P8->OUT &= ~rows[row];
    while(count < 10) {
        if(count == 10){
            break;
        }
        else if(P9->IN & cols[col]) {
            count = 0;
        }
        else if (P9->IN & ~cols[col]){
            count++;
        }
    }
}

void display_key() {

    //unsigned int time = 0;
    if(d > 3){
        d = 0;
    }

    //Number 1
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = nums[0];
    P8->OUT &= ~displays[0];
    waitTime(10);

    //Number 2
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = nums[1];
    P8->OUT &= ~displays[1];
    waitTime(10);

    //Number 3
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = nums[2];
    P8->OUT &= ~displays[2];
    waitTime(10);

    //Number 4
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = nums[3];
    P8->OUT &= ~displays[3];
    waitTime(10);

    release_debouncing();

//    switch(key) {
//        case '1':
//            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
//            P4->OUT = ~0x06;
//            //P8->OUT &= ~displays[i];
//            break;
//        case '2' :
//            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
//            P4->OUT = ~0x5B;
//            P8->OUT &= ~displays[1];
//            break;
//        case '3' :
//            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
//            P4->OUT = ~0x4F;
//            P8->OUT &= ~displays[2];
//            break;
//        case 'A' :
//            P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
//            P4->OUT = 0x88;
//            P8->OUT &= ~displays[3];
//            break;
//    }
}

void press_debouncing() {
    int count = 0;
    while(count < 11) {
        if(count == 10){
            nums[d] = ~keys[row][col];
            display_key();
            break;
        }
        else if(P9->IN & cols[col]) {
            count++;
        }
        else if (P9->IN & ~cols[col]){
            count = 0;
            break;
        }
    }
}

void scan(){
    unsigned int k = 0;

    P9->DIR &= ~BIT3;
    P9->OUT &= ~BIT3;


    while(1){
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P8->DIR |= rows[k];
        P8->OUT &= ~rows[k];
        int i;
        for(i = 0; i < 4; i++){
            if(P9->IN & cols[i]) {
                row = k;
                col = i;
                press_debouncing();
                d++;
            }
        }
        k = k + 1;
        if(k == 4){
           k = 0;
        }
        waitTime(10);
    }
}

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    P8->DIR = 0xff;
    P4->DIR = 0xff;

    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = 0xff;
    P8->OUT &= ~BIT5;
    scan();
}





