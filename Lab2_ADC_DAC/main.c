#include "msp.h"
#include <math.h>
#include <stdio.h>

int ones;
int tens;
int hundreds;
int i = 0;
unsigned int digits[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
uint8_t  sine_wave[256] = {
  0x80, 0x83, 0x86, 0x89, 0x8C, 0x90, 0x93, 0x96,
  0x99, 0x9C, 0x9F, 0xA2, 0xA5, 0xA8, 0xAB, 0xAE,
  0xB1, 0xB3, 0xB6, 0xB9, 0xBC, 0xBF, 0xC1, 0xC4,
  0xC7, 0xC9, 0xCC, 0xCE, 0xD1, 0xD3, 0xD5, 0xD8,
  0xDA, 0xDC, 0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8,
  0xEA, 0xEB, 0xED, 0xEF, 0xF0, 0xF1, 0xF3, 0xF4,
  0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC,
  0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD,
  0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6,
  0xF5, 0xF4, 0xF3, 0xF1, 0xF0, 0xEF, 0xED, 0xEB,
  0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
  0xDA, 0xD8, 0xD5, 0xD3, 0xD1, 0xCE, 0xCC, 0xC9,
  0xC7, 0xC4, 0xC1, 0xBF, 0xBC, 0xB9, 0xB6, 0xB3,
  0xB1, 0xAE, 0xAB, 0xA8, 0xA5, 0xA2, 0x9F, 0x9C,
  0x99, 0x96, 0x93, 0x90, 0x8C, 0x89, 0x86, 0x83,
  0x80, 0x7D, 0x7A, 0x77, 0x74, 0x70, 0x6D, 0x6A,
  0x67, 0x64, 0x61, 0x5E, 0x5B, 0x58, 0x55, 0x52,
  0x4F, 0x4D, 0x4A, 0x47, 0x44, 0x41, 0x3F, 0x3C,
  0x39, 0x37, 0x34, 0x32, 0x2F, 0x2D, 0x2B, 0x28,
  0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18,
  0x16, 0x15, 0x13, 0x11, 0x10, 0x0F, 0x0D, 0x0C,
  0x0B, 0x0A, 0x08, 0x07, 0x06, 0x06, 0x05, 0x04,
  0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03,
  0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x0A,
  0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x13, 0x15,
  0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24,
  0x26, 0x28, 0x2B, 0x2D, 0x2F, 0x32, 0x34, 0x37,
  0x39, 0x3C, 0x3F, 0x41, 0x44, 0x47, 0x4A, 0x4D,
  0x4F, 0x52, 0x55, 0x58, 0x5B, 0x5E, 0x61, 0x64,
  0x67, 0x6A, 0x6D, 0x70, 0x74, 0x77, 0x7A, 0x7D
};

void waitTime(unsigned int time){
    unsigned int i = 0;
    while(i < time){
        i++;
    }
}

//Gets the individual digits to send to the display.
void parse_volts(uint16_t value) {

    int volts = roundf((value * 3.3)/16384 * 100);
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

    //Clock source: DCO, nominal DCO frequency: 48MHz
    CS->KEY = 0x695A; //unlock CS registers(See Figure 6.7)
    CS->CTL0 |= BIT(18) | BIT(16); //DCORSEL frequency range = 48Mhz(See Figure 6.7)
    CS->CTL0 |= BIT(23); //DCOEN, enables DCO oscillator(See Figure 6.7)

    //2.The requested clock signal (SMCLK) is enabled:
    //Clock signal that uses DCO: SMCLK
    CS->CTL1 |= BIT4 | BIT5; //SMCLK source: DCOCLK(See Figure 6.8)
    CS->CLKEN |= BIT3; //Enable SMCLK (SMCLK_EN)(See Figure 6.9)
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
    ADC14->CTL1 |= BIT(16) | BIT(17);

    ADC14->CTL0 |= BIT(0);

    while(ADC14->CTL0 & BIT(16)){

    };

    uint16_t value = ADC14->MEM[0];

    parse_volts(value);

    display();

}

void generate_sine(){

    P10->DIR &= ~(BIT4); //Enables the DAC by pulling /CSA pin low P10.4
    P7->OUT &= sine_wave[i];  //Sets the NDAC's first 8 bits
    P10->DIR &= ~(BIT5);
    P10->DIR |= BIT4;
    if(i < 255){
        i++;
    } else {
        i = 0;
    }
    waitTime(100);


}

void main(void)
{

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	initialize_clock();
	configure_ADC();

	P8->DIR = 0xff;
    P4->DIR = 0xff;

    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = 0xff;
    P8->OUT &= ~BIT5;

    P7->DIR = 0xff;

	while(1) {

	    generate_sine();

	}

}



