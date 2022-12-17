



/*
 * StepperTest.c
 *
 *  Created on: Nov. 20, 2022
 *
 *
 *
 *      Author: colton
 */
#include <msp430.h>

void setup();
void timerSetup();
void stepMotor(int thisStep);
void step(int steps_to_move);
long setSpeed(long whatSpeed);
void halt();

volatile unsigned int micros = 0;
 int direction = 0;            // Direction of rotation
 unsigned long step_delay = 10; // delay between steps, in ms, based on speed
 int number_of_steps;      // total number of steps this motor can take
 int step_number = 0;          // which step the motor is on
 unsigned long last_step_time = 0; // time stamp in us of when the last step was taken
 const int revolution = 2038;


void main(void)
{
    setup();

while(1)
{
   long speed = setSpeed(10); //takes in a long returns a long
   //step(revolution);
   halt();
}
}

#pragma vector=TIMER0_A0_VECTOR

__interrupt void CCR0_ISR (void)

{
 micros =+ 1;
 P1OUT ^= 0b01000000;              // toggle P1.0

}


void setup()
{
       WDTCTL = WDTPW + WDTHOLD;       // stop watchdog timer

       BCSCTL2 |= DIVS_3; //should give 2 Mhz smclk
       P1DIR |= BIT6 + BIT1 + BIT0;                 //configure P1.0 and P1.1 and P1.6 as output
       P2DIR |= BIT5 + BIT4;                 //configure P2.4 and P2.5 as output

       P1OUT |= 0b00000000;                 //turn outputs off
       P2OUT |= 0b00000000;                 //turn outputs off

       //P1.0 = coil 3
       //P1.1 = coil 4
       //P2.4 = coil 1
       //P2.5 = coil 2


        TACCR0 = 10000; //max is 2^16 -1 = 65535
        TACCTL0 |= CCIE;
        TACTL |= MC_1 + TASSEL_2 + ID_1;  // Set Mode -> Up Count, Clock -> SMCLK, clk divide 2/2 = 1Mhz

        __bis_SR_register(GIE); // Enable CPU Interrupt

}



void stepMotor(int thisStep)
{
    switch(thisStep)
    { //XXXX refers to coils
    case 0: //1010
        P1OUT |= 0b00000001;                 //coil 3 on 4 off
        P2OUT |= 0b00001000;                 //coil 1 on 2 off
        break;

    case 1://0110
        P1OUT |= 0b00000001;                 //coil 3 on 4 off
        P2OUT |= 0b00010000;                 //coil 2 on 1 off
        break;
    case 2://0101
        P1OUT |= 0b00000010;                 //coil 4 on 3 off
        P2OUT |= 0b00010000;                 //coil 2 on 1 off
        break;
    case 3://1001
        P1OUT |= 0b00000010;                 //coil 4 on 3 off
        P2OUT |= 0b00001000;                 //coil 1 on 2 off
        break;
    }
}
long setSpeed(long whatSpeed)
{
 return step_delay = 60L * 1000L * 1000L / number_of_steps / whatSpeed;
}

void step(int steps_to_move)
{
  int steps_left = abs(steps_to_move);  // how many steps to take

  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) { direction = 1; }
  if (steps_to_move < 0) { direction = 0; }


  // decrement the number of steps, moving one step each time:
  while (steps_left > 0)
  {
    unsigned long now = micros;
    // move only if the appropriate delay has passed:
    if (now - last_step_time >= step_delay)
    {
      // get the timeStamp of when you stepped:
      last_step_time = now;
      // increment or decrement the step number,
      // depending on direction:
      if (direction == 1)
      {
        step_number++;
        if (step_number == number_of_steps) {
          step_number = 0;
        }
      }
      else
      {
        if (step_number == 0) {
          step_number = number_of_steps;
        }
        step_number--;
      }
      // decrement the steps left:
      steps_left--;
      // step the motor to step number 0, 1, ..., {3 or 10}

      stepMotor(step_number % 4);
    }
  }
}
void halt()
{
    P1OUT |= 0b00000000;                 //coil 4 on 3 off
    P2OUT |= 0b00000000;                 //coil 1 on 2 off
}

