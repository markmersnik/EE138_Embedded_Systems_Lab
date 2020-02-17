#include "msp.h"
#include <time.h>
#include <stdbool.h>

/**
 * main.c
 */

unsigned int d = 0;
unsigned int k = 0;
bool code_entered = false;
bool locked= false;
bool passcode_required = false;
bool reset = false;
unsigned int wrong_passcode = 0;
unsigned int row = 0;
unsigned int col = 0;
unsigned int passcode[] = {0x01, 0x01, 0x01, 0x01};
unsigned int blank[] = {0xff, 0xff, 0xff, 0xff};
unsigned int nums[] = {0xff, 0xff, 0xff, 0xff};
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

bool checkCode() {
    unsigned int j;
    for (j = 0; j < 4; j++) {
        if(nums[j] != passcode[j]) {
            return false;
        }
    }
    return true;
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

void lockdown(){
    unsigned int i = 0;
    //10 second lockdown period
    while(i < 3000000){
        //Number 1
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x40; //Adjusts what turns on.
        P8->OUT &= ~BIT5;
        waitTime(10);

        //Number 2
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x40;
        P8->OUT &= ~BIT4;
        waitTime(10);

        //Number 3
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x40;
        P8->OUT &= ~BIT3;
        waitTime(10);

        //Number 4
        P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
        P4->OUT = ~0x40;
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
}

void press_debouncing() {

    int count = 0;

    while(count < 11) {

        if(count == 10){
            if(d > 3){
                code_entered = true;
                d = 0;
            }
            //Normal mode
            if(~locked) {
                if(row == 3 && col == 2 && ~code_entered) {
                    P2->OUT &= ~BIT5;
                    waitTime(1500000);
                    P2->OUT |= BIT5;
                    reset = true;
                }
                else if(row == 3 && col == 2 && code_entered) {
                    locked = true;
                    int j;
                    for(j = 0; j < 4; j++) {
                        passcode[j] = nums[j];
                    }
                    reset = true;
                }
            }
            else {
                if(row == 3 && col == 2 && ~code_entered) {
                    reset = true;
                }
                else if(row == 3 && col == 2 && code_entered) {
                    if(checkCode()){
                        int j;
                        for (j = 0; j < 4; j++) {
                            passcode[j] = 0x01;
                        }
                        P2->OUT &= ~BIT5;
                        waitTime(1500000);
                        locked = false;
                    }
                    else{
                        wrong_passcode++;
                    }
                    reset = true;
                }
            }
            code_entered = false;
            nums[d] = ~keys[row][col];
            release_debouncing();
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

    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = 0xff;
    P8->DIR |= rows[k];
    P8->OUT &= ~rows[k];

    int i;
    for(i = 0; i < 4; i++){
        if(P9->IN & cols[i]) {
            row = k;
            col = i;
            press_debouncing();
            if(reset) {
                int j;
                for (j = 0; j < 4; j++) {
                    nums[j] = 0xff;
                }
                d = 0;
                reset = false;
            }
            else {
                d++;
            }
        }
    }

    k = k + 1;

    if(k == 4){
       k = 0;
    }

    display_key();
}

void locked_mode(){

    unsigned int i = 0;

    locked = false;
    while(i < 1500000) {

        scan();
        i++;

    }
    locked = true;

    passcode_required = true;

    while(locked) {
        if(wrong_passcode > 4) {
            lockdown();
            wrong_passcode = 0;
        }
        scan();
    }
}

void normal_mode(){

    P2->DIR |= BIT5;

    while(1) {

        passcode_required = false;

        while(~locked) {

            scan();

        }

        locked_mode();
    }
}

void test_lockbox(){

    while(1){

        P2->OUT |= BIT5; //de-energizes (locks) the solenoid

        waitTime(300000); //300000 is one second

        P2->OUT &= ~BIT5; //energizes (unlocks) the solenoid

        waitTime(300000);
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

    P9->DIR &= ~BIT3;
    P9->OUT &= ~BIT3;

    normal_mode();

}





