/*
 * StepperMotor.c
 *
 *  Created on: Dec. 27, 2022
 *      Author: Colton
 */


//Get NRF working

#include <msp430.h>



void main(void)
{
    //run initialization function for all devices
    Setup();


    //infinite run loop
    while(1)
    {
        //check for inputs


        //run motor
        //etc


    }
}



void Setup()
{
       //Watchdog timer and Clock initialization
       WDTCTL = WDTPW + WDTHOLD;                //stop watchdog timer
       BCSCTL2 |= DIVS_3;                       //should give 2 Mhz smclk
       //Output declaration
       P1DIR |= BIT5 + BIT4;                    //configure P1.4 and P1.5 as output for motor
       P2DIR |= BIT1 + BIT0;                    //configure P2.1 and P2.0 as output for motor

       P1OUT &= ~BIT5 + ~BIT4;                  //turn outputs off so motor is initially not operating
       P2OUT &= ~BIT1 + ~BIT0;                  //turn outputs off so motor is initially not operating
       //Input declaration
       P1DIR &= ~BIT0;
      // P2DIR &=



       //coil configuration:

       //coil 1 P2.1
       //coil 2 P2.0
       //coil 3 P1.5
       //coil 4 P1.4
}
