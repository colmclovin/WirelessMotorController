/*
 * StepperMotor.c
 *
 *  Created on: Dec. 27, 2022
 *      Author: Colton
 */

#include <msp430.h>

void setup();

void main(void)
{
    setup();

while(1)
{

//delay >1950
    __delay_cycles(1950);
    P1OUT = 0b00000000;
    P2OUT = BIT1 + BIT0;                               //1,2
    __delay_cycles(1950);


    P1OUT = BIT5;
    P2OUT = BIT0;                 //2,3
    __delay_cycles(1950);
    P1OUT = BIT5 + BIT4;
    P2OUT = 0b00000000;                 //3,4

    __delay_cycles(1950);
    P1OUT = BIT4;
    P2OUT = BIT1;                 //4,1
    __delay_cycles(1950);

     //P1OUT = BIT5 + BIT4;                 //Coil 3 and Coil 4
     //P2OUT = BIT1 + BIT0;                 // Coil 1 and Coil 2

}
}

#pragma vector=TIMER0_A0_VECTOR

__interrupt void CCR0_ISR (void)
{



}


void setup()
{
       WDTCTL = WDTPW + WDTHOLD;       // stop watchdog timer

       BCSCTL2 |= DIVS_3; //should give 2 Mhz smclk
       P1DIR |= BIT5 + BIT4;
       P2DIR |= BIT1 + BIT0;                 //configure P2.4 and P2.5 as output

       P1OUT |= 0b00000000;                 //turn outputs off
       P2OUT |= 0b00000000;                 //turn outputs off

       //coil 1 P2.1
       //coil 2 P2.0
       //coil 3 P1.5
       //coil 4 P1.4


}



