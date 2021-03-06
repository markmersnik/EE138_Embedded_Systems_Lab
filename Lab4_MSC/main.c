#include "msp.h"
#include <math.h>
#include <stdio.h>


volatile int count = 0;
int rpm;
int ones;
int tens;
int hundreds;
int thousands;
int points = 64;
int i = 0;
unsigned int bits_for_db[] = {BIT0, BIT1, BIT2, BIT3,
                              BIT4, BIT5, BIT6, BIT7,
                              BIT0, BIT1, BIT2, BIT3};
unsigned int digits[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
unsigned int new_sine[190];
unsigned int value;
int period = 1875;
int pwm_period = 255;
uint8_t duty1value;
uint8_t duty2value;

float y[] = {0, 0, 0};
float x[] = {0, 0, 0};


void waitTime(unsigned int time){
    unsigned int i = 0;
    while(i < time){
        i++;
    }
}

void parse_rpm(){

    if(rpm < 0) {
        P5->OUT |= 0x01;
        rpm = -1*rpm;
    }
    else {
        P5->OUT &= ~0x01;
    }
    double hold = rpm * 0.8;
    rpm = (int) hold;
    ones = rpm%10;
    rpm = rpm/10;
    tens = rpm%10;
    rpm = rpm/10;
    hundreds = rpm%10;
    rpm = rpm/10;
    thousands = rpm%10;

}

//Displays our individual digits on the display.
void display() {

    //Number 1
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT =  ~(digits[thousands]);
    P8->OUT &= ~BIT5;
    waitTime(2000);

    //Number 2
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = ~(digits[hundreds]);
    P8->OUT &= ~BIT4;
    waitTime(2000);

    //Number 3
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = ~digits[tens];
    P8->OUT &= ~BIT3;
    waitTime(2000);

    //Number 4
    P8->OUT |= (BIT5 | BIT4 | BIT3| BIT2);
    P4->OUT = ~digits[ones];
    P8->OUT &= ~BIT2;
    waitTime(2000);

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
    CS->CTL0 |= BIT(23); //DCOEN, enables DCO oscillator(See Figure 6.7))

    //2.The requested clock signal (SMCLK) is enabled:
    //Clock signal that uses DCO: SMCLK
    CS->CTL1 |= BIT(4) | BIT(5); //SMCLK source: DCOCLK(See Figure 6.8)
    CS->CTL1 |= BIT(28) | BIT(29) | BIT(30); // divides clock by 128 making about 375kHz
    CS->CLKEN |= BIT(3); //Enable SMCLK (SMCLK_EN)(See Figure 6.9)
    CS->KEY = 0x0; //lock CS registers(See Figure 6.7)

}

void configure_ADC() {

    ADC14->CTL0 &= ~(BIT(1)); //STEP 1: sets ADC14ENC bit to 0

    ADC14->CTL0 |= BIT(21); //STEP 2: sets clock signal source to SMCLK (bits 21-19)
    ADC14->CTL0 |= BIT(31); //PDIV clock pre divider value (bits 31-30)
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

void read_adc() {
    //ADC14->CTL1 |= BIT(16); //sets the memory address to MEM[0]

    ADC14->CTL0 |= BIT(0); //starts the sample-and-conversion

    while(ADC14->CTL0 & BIT(16)){ //Bit 16 is ADC14BUSY which indicates active sample and conversion operation.

    };

    value = ADC14->MEM[0];

    duty1value = (value*(pwm_period))/16384;
    if(duty1value > (pwm_period-1)) {
        duty1value = pwm_period-1;
    }
    duty2value = pwm_period - duty1value;

}

void output_dac(float outputValue){

    P10->OUT &= ~(BIT4); // Step 1: Enables the DAC by pulling /CSA pin low P10.4

    P7->OUT = (int) outputValue & 0xff;
    P10->OUT = (int) (outputValue);

    P10->OUT &= ~(BIT5);
    P10->OUT |= BIT4;

}


void configure_PWM(uint16_t duty1, uint16_t duty2){

    P2->DIR |= 0x30; // P2.4, P2.5 output
    P2->SEL0 |= 0x30; // P2.4, P2.5 Timer0A functions
    P2->SEL1 &= ~0x30; // P2.4, P2.5 Timer0A functions
    TIMER_A0->CCTL[0] = 0x0080; // CCI0 toggle
    TIMER_A0->CCTL[0] &= ~(0x0010); // CCI0 toggle
    TIMER_A0->CCR[0] = pwm_period; //CCR0 is set to 0xff which is our period (375kHz/255)
    TIMER_A0->EX0 = 0x0000; // divide by 1
    TIMER_A0->CCTL[1] = 0x0060; // CCR1 set/reset
    TIMER_A0->CCR[1] = duty1; // CCR1 duty cycle is duty1/period
    TIMER_A0->CCTL[2] = 0x0060; // CCR2 set/reset
    TIMER_A0->CCR[2] = duty2; // CCR2 duty cycle is duty2/period
    TIMER_A0->CTL = 0x0210; // SMCLK=375kHz, divide by 1, up mode

}

void run_PWM(uint16_t duty1, uint16_t duty2) {
    TIMER_A0->CCR[1] = duty1; // CCR1 duty cycle is duty1/period
    TIMER_A0->CCR[2] = duty2; // CCR2 duty cycle is duty2/period
}

void start_motor() {
    //P2.4 (U) and P2.5 (V) control the motor.
    //Average voltage across the motor V = 24(D_u - D_v).
    read_adc();
    run_PWM(duty1value, duty2value);

}

//Timer_A register on page 797
void TimerA2_Init(){
    TIMER_A2->CTL = 0x0200;  //sets clock to SMCLK
    TIMER_A2->CCTL[0] = 0x0010;  //enables the interrupt
    TIMER_A2->CCR[0] = period;   // input of 1875 as divider (counts up to this every cycle) making 200Hz
    NVIC->IP[3] = (NVIC->IP[3]&0xFFFFFF00)|0x00000060; // priority 2
    NVIC->ISER[0] |= 0x00001000;   // enable interrupt 12 in NVIC
    TIMER_A2->CTL |= 0x0014;      // reset and start Timer A in up mode
}

//2 pins, p5.3 (phase B) and p3.6 (phase A)
//the code assumes encoder count increase when turning CW and decrease when turning CCW
//count is stored in a counter variable

void Interrupt_Init() {
    P3->SEL0 &= ~0x40;  //Function group set to 0 for IO
    P3->SEL1 &= ~0x40;  //Function group set to 0 for IO
    P3->DIR &= ~0x40;   //P3.6 set to input
    P3->IES &= ~0x40;   //Interrupt Edge Selector set to 0 for rising edge
    P3->IFG &= ~0x40;   //Interrupt Flag Register cleared (if its 1 then interrupt was triggered, has to be manually reset back to 0 each time)
    P3->IE |= 0x40;     //Interrupt Enabled registers set to 1 (enabled)
    NVIC->IP[9] = (NVIC->IP[9]&0x00FFFFFF)|0x40000000; //set to priority 3
    NVIC->ISER[1] |= 0x00000020; //page 116 in the manual. Sets IRQ 37 enabling Port3_IRQ

    P5->SEL0 &= ~0x08;  //Function group set to 0 for IO
    P5->SEL1 &= ~0x08;  //Function group set to 0 for IO
    P5->DIR &= ~0x08;   //P5.3 set to input
    P5->IES &= ~0x08;   //Interrupt Edge Selector set to 0 for rising edge
    P5->IFG &= ~0x08;   //Interrupt Flag Register cleared (if its 1 then interrupt was triggered)
    P5->IE |= 0x08;     //Interrupt Enabled registers set to 1 (enabled)

    //NVIC registers on page 115
    NVIC->IP[9] = (NVIC->IP[9]&0x00FFFFFF)|0x40000000; //set to priority 3
    NVIC->ISER[1] |= 0x00000080; //page 116 in the manual. Sets IRQ 39 enabling Port5_IRQ
}

void PORT3_IRQHandler(void)
{
    //check phase B logic state. if it is high, then decrease counter variable by 1
    //else if it is low, then increase counter variable by 1
    if(P5->IN & 0x08) {
        count--;
    }
    else {
        count++;
    }
    P3->IFG &= ~0x40;
}

void PORT5_IRQHandler(void)
{
    // check phase A logic state. If Phase A pin is High, increase the counter by 1.
    //else if it is low, then decrease counter variable by 1
    if(P3->IN & 0x40) {
        count++;
    }
    else {
        count--;
    }
    P5->IFG &= ~0x08;
}

void TA2_0_IRQHandler(void) {
    rpm = (count * 200 * 60)/800;
    y[1] = (0.9968)*y[0] + (0.004992)*rpm;
    y[0] = y[1];
    count = 0;
    TIMER_A2->CCTL[0] &= ~(BIT0); //clear the int flag
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
    P10->DIR = 0xff;

    P5->DIR |= 0x01;    //sets P5.0 LED to output

//    P6->SEL0 &= ~0x01; //clears bit 0
//    P6->SEL1 &= ~0x01; //
//    P6->DIR &= ~BIT0; //set direction to input
//    P6->OUT |= BIT0; //sets input to high (BIT 0 is a 1)
//    P6->REN |= BIT0; //enable pull up or pull down
//    P2->IES |= BIT2;  //  P2.2 falling edge interrupt
//    NVIC->ISER[1] |= 1 << (36 & 0x001F);
//
//    NVIC->ISER[0] |= BIT(10); //enable interrupt
//    NVIC->IP[10] &= ~(BIT7 | BIT6 | BIT5); //setting timer priority to 0

    //New Lab 4 program to initialize interrupts
    read_adc();
    configure_PWM(duty1value, duty2value);
	Interrupt_Init();
	TimerA2_Init();

	while(1) {
	    start_motor();
	    parse_rpm();
	    display();
	}

}
