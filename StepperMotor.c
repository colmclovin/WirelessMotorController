/*
 * StepperMotor.c
 *
 *  Created on: Dec. 27, 2022
 *      Author: Colton
 */


//get motor working (DONE)
//get motor working     through a button
//turn external LEDs on
//Get NRF working
//put it all together

#include <msp430.h>

void motorSetup();
void motorCW();


void main(void)
{
    //run initialization function for all devices
    motorSetup();


    //infinite run loop
    while(1)
    {
        //check for inputs


        //run motor
        motorCW();
        //etc


    }
}



void motorSetup()
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

void motorCW()
{
    //CLOCKWISE MOTOR ROTATION
//delay >1950
    //1,2
    __delay_cycles(3000);
    P1OUT &= ~BIT5 + ~BIT4;//coil 3 and 4 off
    P2OUT |= BIT1 + BIT0; //1,2 on
    //2,3
    __delay_cycles(3000);
    P1OUT &= ~BIT4; // coil 4 off
    P2OUT &= ~BIT1; //coil 1 off
    P1OUT |= BIT5; // coil 3 on
    P2OUT |= BIT0; // coil 2 on
    //3,4
    __delay_cycles(3000);
    P1OUT |= BIT5 + BIT4; //coil 3 and 4 on
    P2OUT &= ~BIT0; //coil 2 off

    P2OUT &= ~BIT1 + ~BIT0; // coil 1 and 2 off

    //4,1
    __delay_cycles(3000);
    P1OUT &= ~BIT5; //coil 3 off
    P2OUT &= ~BIT0; //coil 2 off
    P1OUT |= BIT4; //coil 4 on
    P2OUT |= BIT1; //coil 1 on
    __delay_cycles(3000);

 //P1OUT = BIT5 + BIT4;                 //Coil 3 and Coil 4
 //P2OUT = BIT1 + BIT0;                 // Coil 1 and Coil 2
}
void motorStop()
{
    P1OUT &= ~BIT5; //coil 3 off
    P2OUT &= ~BIT0; //coil 2 off
    P1OUT &= ~BIT4; //coil 4 off
    P2OUT &= ~BIT1; //coil 1 off

}
void SPIset(){
    UCB0CTL1 |= UCSWRST; //PUT B0 into SW reset
    UCB0CTL1 |= UCSSEL_2; // CHOOSE SMCLK CLOCK
    UCB0BR0 = 10; //SET PRESCLAR TO 10 TO get clock = 100kHz
    UCB0CTL0 |= UCSYNC; //put B0 in SPI mode
    UCB0CTL0 |= UCMST; //put into SPI master

    //configuring the ports
    P1SEL2 |= BIT5;    //P1.5 SCLK (11)
    P1SEL |=  BIT5;

    P1SEL2 |= BIT7;    //P1.7 SIM0 (11)
    P1SEL |= BIT7;

    P1SEL2 |= BIT6;    //P1.6 SOM0 (11)
    P1SEL |= BIT6;


    UCB0CTL1 &= ~UCSWRST; // TAKE B0 out of reset mode

    int i;
     while(1){
         UCB0TXBUF = 0x4D;
         for(i=0; i<10000;i+1){

         }
     }

}

