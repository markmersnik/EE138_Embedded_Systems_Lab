#include "msp.h"
#include <math.h>
#include <stdio.h>

int ones;
int tens;
int hundreds;
int points = 64;
int i = 0;
unsigned int bits_for_db[] = {BIT0, BIT1, BIT2, BIT3,
                              BIT4, BIT5, BIT6, BIT7,
                              BIT0, BIT1, BIT2, BIT3};
unsigned int digits[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
unsigned int new_sine[190];
unsigned int value;
uint8_t duty1value;
uint8_t duty2value;

void waitTime(unsigned int time){
    unsigned int i = 0;
    while(i < time){
        i++;
    }
}

//Gets the individual digits to send to the display.
void parse_volts(uint16_t value) {

    int volts = roundf((value * 3.3)/0x3fce * 100);
    ones = volts%10;
    volts = volts/10;
    tens = volts%10;
    volts = volts/10;
    hundreds = volts%10;

}

//Displays our individual digits on the display.
void display() {
    //Number 1
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = ~0x00;
    P8->OUT &= ~BIT5;
    waitTime(100);

    //Number 2
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = ~(digits[hundreds] | 0x80); //Add the decimal point to the display output.
    P8->OUT &= ~BIT4;
    waitTime(100);

    //Number 3
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = ~digits[tens];
    P8->OUT &= ~BIT3;
    waitTime(100);

    //Number 4
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = ~digits[ones];
    P8->OUT &= ~BIT2;
    waitTime(100);

    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
}

void initialize_clock() {

    //Set power level for the desired clock frequency
    while(PCM->CTL1 & 0x00000100); //wait for PCM to become not busy
    uint32_t key_bits = 0x695A0000;
    uint32_t AM_LDO_VCORE1_bits = 0x00000001; //AMR Active Mode Request - 01b = AM_LDO_VCORE1
    PCM->CTL0 = key_bits | AM_LDO_VCORE1_bits; //unlock PCM register and set power mode
    while(PCM->CTL1 & 0x00000100); //wait for PCM to become not busy
    PCM->CTL0 &= 0x0000FFFF; //lock PCM register again

    //Flash read wait state number change;
    FLCTL->BANK0_RDCTL &= ~(BIT(12) | BIT(13) | BIT(14) | BIT(15)); //reset bits
    FLCTL->BANK0_RDCTL |= BIT(12); //bit 12 - 15: wait state number selection. 0001b = 1 wait states
    FLCTL->BANK1_RDCTL &= ~(BIT(12) | BIT(13) | BIT(14) | BIT(15)); //reset bits
    FLCTL->BANK1_RDCTL |= BIT(12); //bit 12 - 15: wait state number selection. 0001b = 1 wait states

    //Clock source: DCO, nominal DCO frequency: 1.5MHz
    CS->KEY = 0x695A; //unlock CS registers(See Figure 6.7)
    CS->CTL0 &= ~(BIT(16) | BIT(17) | BIT(18)); //DCORSEL frequency range = 1.5Mhz(See Figure 6.7)
    CS->CTL0 |= BIT(23); //DCOEN, enables DCO oscillator(See Figure 6.7)

    //2.The requested clock signal (SMCLK) is enabled:
    //Clock signal that uses DCO: SMCLK
    CS->CTL1 |= BIT(4) | BIT(5); //SMCLK source: DCOCLK(See Figure 6.8)
    CS->CTL1 &= ~(BIT(28) | BIT(29) | BIT(30)); // divides clock by 128 making about 12kHz
    CS->CLKEN |= BIT(3); //Enable SMCLK (SMCLK_EN)(See Figure 6.9)
    CS->KEY = 0x0; //lock CS registers(See Figure 6.7)

}

void configure_ADC() {

    ADC14->CTL0 &= ~(BIT(1)); //STEP 1: sets ADC14ENC bit to 0

    ADC14->CTL0 |= BIT(21); //STEP 2: sets clock signal source to SMCLK (bits 21-19)
    ADC14->CTL0 |= BIT(31); //PDIV clock pre-divider value (bits 31-30)
    ADC14->CTL0 |= BIT(23); //DIVx clock divider value (bits 24-22)

    ADC14->CTL0 &= ~(BIT(27) | BIT(28) | BIT(29)); //STEP 3: SHSx set to 000 for software conversion.

    ADC14->CTL0 &= ~(BIT(18) | BIT(17)); //STEP 4: CONSEQX set to 00 for single-channel-single-conversion.

    ADC14->MCTL[0] |= BIT(2); //STEP 5: Set memory output to MEM[5]

    ADC14->CTL0 |= BIT(26); //STEP 6:
    ADC14->CTL0 |= BIT(8) | BIT(9);

    P5->SEL0 |= BIT(1);
    P5->SEL1 |= BIT(1);

    ADC14->CTL0 |= BIT(4); //STEP 8: sets ADC14ON bit to 1, turning ADC on.

    ADC14->CTL0 |= BIT(1); //STEP 9: sets ADC14ENC bit back to 1

    P9->DIR |= BIT(4);
    P9->OUT |= BIT(4);

}

void read() {
    //ADC14->CTL1 |= BIT(16); //sets the memory address to MEM[0]

    ADC14->CTL0 |= BIT(0); //starts the sample-and-conversion

    while(ADC14->CTL0 & BIT(16)){ //Bit 16 is ADC14BUSY which indicates active sample and conversion operation.

    };

    value = ADC14->MEM[0];

    duty1value = roundf ((value*256)/16384);
    duty2value = 0xff - duty1value;

    parse_volts(value);

    display();

}

void convert_sine_values(){
    int j;
    float temp = 0;
    for(j = 0; j < 190; j++){
        new_sine[j] = (int)((sin(temp) + 1) * 1848 + 200);
        temp = temp + (2 * M_PI)/190;
    }
}

void generate_sine(){
    P10->OUT &= ~(BIT4); // Step 1: Enables the DAC by pulling /CSA pin low P10.4

    P7->OUT = new_sine[i] & 0xff;
    P10->OUT = 0xf & (new_sine[i]>>8);

    P10->OUT &= ~(BIT5);
    P10->OUT |= BIT4;
    if(i < 190){
        i = i + 1;
    } else {
        i = 0;
    }
}


void configure_PWM(uint16_t period, uint16_t duty1, uint16_t duty2){
 P2->DIR |= 0x30; // P2.4, P2.5 output
 P2->SEL0 |= 0x30; // P2.4, P2.5 Timer0A functions
 P2->SEL1 &= ~0x30; // P2.4, P2.5 Timer0A functions
 TIMER_A0->CCTL[0] = 0x0080; // CCI0 toggle
 TIMER_A0->CCR[0] = period; // Period is 2*period*8*83.33ns is 1.333u*period
 TIMER_A0->EX0 = 0x0000; // divide by 1
 TIMER_A0->CCTL[1] = 0x0040; // CCR1 toggle/reset
 TIMER_A0->CCR[1] = duty1; // CCR1 duty cycle is duty1/period
 TIMER_A0->CCTL[2] = 0x0040; // CCR2 toggle/reset
 TIMER_A0->CCR[2] = duty2; // CCR2 duty cycle is duty2/period
 TIMER_A0->CTL = 0x02F0; // SMCLK=12MHz, divide by 8, up-down mode
}

void start_motor() {
    //P2.4 (U) and P2.5 (V) control the motor.
    //Average voltage across the motor V = 24(D_u - D_v).
    read();

    configure_PWM(0xff, duty1value, duty2value);


}

void main(void)
{

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    initialize_clock();
    configure_ADC();

    P8->DIR = 0xff;
    P4->DIR = 0xff;

    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = 0xff;
    P8->OUT &= ~BIT5;

    P7->DIR = 0xff;
    P10->DIR = 0xff;

    while(1) {

        start_motor();

    }

}

