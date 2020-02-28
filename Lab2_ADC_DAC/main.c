#include "msp.h"
#include <stdio.h>

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

    printf("%d\n", value);
}

void main(void)
{

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	initialize_clock();
	configure_ADC();

	while(1) {
	    read();
	}

}



