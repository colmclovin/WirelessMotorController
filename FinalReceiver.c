/*
  Copyright (C) 2022 Selim Gullulu <drselim2021@gmail.com>
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.
  */

#include <msp430.h>
#include "nrf24l01.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MOSI BIT2  //   P1.2
#define MISO BIT1  //   P1.1
#define SCLK BIT4  //   P1.4
#define CE BIT7    //   P1.7
#define CSN BIT3   //   P1.3

#define COIL1 BIT5 //1.5
#define COIL2 BIT0 //2.0
#define COIL3 BIT1 //2.1
#define COIL4 BIT2 //2.2
#define LED BIT0



 int i;
 int j;
 int k;
 int l; //for pload2, so many indexes..
 int x;
 int pd; //payload func index 0-15
 int pd_i;  //payload func i index 0-7
 int pd_x;
int pipe_nr; //4bytes = 32 bits
int pyld1[8]; //1st 16 bytes in rx fifo
int pyld2[8]; //2nd 16 bytes in rx fifo

unsigned char status_reg;
unsigned char read_reg[5];
char buf[5];
char pipe_nr_chr[5]; //8bits*5 = 40 bits


unsigned char read_reg_CONFIG[1];
unsigned char read_reg_EN_AA[1];
unsigned char read_reg_EN_RXADDR[1];
unsigned char read_reg_SETUP_AW[1];
unsigned char read_reg_SETUP_RETR[1];
unsigned char read_reg_RF_CH[1];
unsigned char read_reg_RF_SETUP[1];
unsigned char read_reg_STATUS[1];
unsigned char read_reg_OBSERVE_TX[1];
unsigned char read_reg_CD[1];
unsigned char read_reg_RX_ADDR_P0[5];  //5 BYTES
unsigned char read_reg_RX_ADDR_P1[5];  //5 BYTES
unsigned char read_reg_RX_ADDR_P2[1];
unsigned char read_reg_RX_ADDR_P3[1];
unsigned char read_reg_RX_ADDR_P4[1];
unsigned char read_reg_RX_ADDR_P5[1];
unsigned char read_reg_TX_ADDR[5];  //5 BYTES
unsigned char read_reg_RX_PW_P0[1];
unsigned char read_reg_RX_PW_P1[1];
unsigned char read_reg_RX_PW_P2[1];
unsigned char read_reg_RX_PW_P3[1];
unsigned char read_reg_RX_PW_P4[1];
unsigned char read_reg_RX_PW_P5[1];
unsigned char read_reg_FIFO_STATUS[1];
unsigned char read_PAYLOAD[32];

unsigned char clr_status[1]={0x70}; //clr status reg

unsigned char rf_setupregister[1]={0b00000001};  //Data Rate -> '0' (1 Mbps) & PA_MIN
unsigned char configregister[1]={0b00001111};  //CRC '1'-> (2 bytes)  & Power ON & Enable CRC & RECEIVER -------1
unsigned char rf_chanregister[1]={0b01001100};  //Channel '1001100'
unsigned char address[6]="00001";  //write to RX_ADDR_P0 and TX_ADDR
unsigned char setup_retr_register[1]={0b01011111};  //retry values
unsigned char en_aa_register[1]={0b00111111};
unsigned char rx_pw_register[1]={0b00100000};  //RX_ payload width register -->32



void SCLK_Pulse (void);  //To create a clock pulse high low
void Send_Bit (unsigned int value);     //For sending 1 or zero
void CE_On (void);  //Chip enable
void CE_Off (void);  //Chip disable
void CSN_On (void);     //CSN On
void CSN_Off (void);    //CSN Off
void Write_Byte (int content);
void Instruction_Byte_MSB_First (int content);
void Read_Byte_MSB_First(int index, unsigned char regname[]);
void Write_Byte_MSB_First(unsigned char content[], int index2);
void Write_Payload_MSB_First(int pyld[], int index3);
void motorStop(void);
void motorCW(void);
void motorCCW(void);
void main(void)
    {

    __delay_cycles(100); //power on reset

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P2DIR |= COIL2 + COIL3 + COIL4;
    P2OUT &= 0x00;
    P1DIR |= MOSI + SCLK + CE + CSN + LED + COIL1;  //Output Pins
    P1DIR &= ~MISO;
    P1OUT &= ~COIL1;


    CE_Off();
    CSN_On();
    /************************
    **CONFIGURING REGISTERS**
    *************************/
    //EN_AA  -- enabling AA in all pipes
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | EN_AA);
    Write_Byte_MSB_First(en_aa_register,1);
    CSN_On();
    //RF_SETUP
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RF_SETUP);
    Write_Byte_MSB_First(rf_setupregister,1);
    CSN_On();
    //RX_ADDR_P0
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_ADDR_P0);
    Write_Byte_MSB_First(address,5); // write 5 bytes address
    CSN_On();
    //TX_ADDR
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | TX_ADDR);
    Write_Byte_MSB_First(address,5); // write 5 bytes address
    CSN_On();
    //RF_CH
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RF_CH);
    Write_Byte_MSB_First(rf_chanregister,1);
    CSN_On();
    //SETUP_RETR
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | SETUP_RETR);
    Write_Byte_MSB_First(setup_retr_register,1);
    CSN_On();
    //RX_PW0
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P0);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW1
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P1);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW2
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P2);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW3
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P3);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW4
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P4);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW4
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P5);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //CONFIG
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | CONFIG);
    Write_Byte_MSB_First(configregister,1);
    CSN_On();
    /****************************
    **END CONFIGURING REGISTERS**
    *****************************/
    __delay_cycles(2000);  //start_up >1.5 ms

    while(1){
        CE_On();
        __delay_cycles(150); //settling RX
        CSN_Off();
        Instruction_Byte_MSB_First(NOP);  //to get the status reg..
        CSN_On();
        if ((status_reg & BIT6) == 0x40){
            CE_Off();
            CSN_Off();
            Instruction_Byte_MSB_First(R_RX_PAYLOAD);
            Read_Byte_MSB_First(32,read_PAYLOAD);
            CSN_On();
            pipe_nr = status_reg & BIT4;


            ltoa(pipe_nr,pipe_nr_chr,10); //ti recommended

            j=0;
            l=0;
            for (i=0;i<=14;i+=2){
                pyld1[j]=read_PAYLOAD[i] | (read_PAYLOAD[i+1] << 8);
                ltoa(pyld1[j],buf,10);

                j++;

            }

            for (i=16;i<=30;i+=2){
                            pyld2[l]=read_PAYLOAD[i] | (read_PAYLOAD[i+1] << 8);
                            ltoa(pyld1[j],buf,10);
                            l++;
                        }

            CSN_Off();
            Instruction_Byte_MSB_First(W_REGISTER | STATUS);
            Write_Byte_MSB_First(clr_status,1);
            CSN_On();


            if(pyld1[0] == 1)
            {//open

                P1OUT |= LED;
                int v;
                for(v=0;v<1000;v++){
                motorCW();

                }
                __delay_cycles(1000);

                motorStop();
                __delay_cycles(1000);




            }
            else if(pyld1[0] == 0)
            {//close
                P1OUT &= ~LED;
                int v;
                                for(v=0;v<1000;v++){
                motorCCW();
                                }
                motorStop();
                __delay_cycles(1000);



            }
        }

    }
}
void SCLK_Pulse (void)
{
  P1OUT |= SCLK;//set high with OR 1
  P1OUT ^= SCLK;//toggle with XOR 1
}
void Send_Bit (unsigned int value)
{
    if (value != 0){
        P1OUT |= MOSI;}
    else {
        P1OUT &= ~MOSI;
    }
}
void CE_On (void)
{
    P1OUT |= CE;
}

void CE_Off (void)
{
    P1OUT &= ~CE;
}
void CSN_On (void)
{
    P1OUT |= CSN;
}
void CSN_Off (void)
{
    P1OUT &= ~CSN;
}
void Instruction_Byte_MSB_First(int content)
{

    for (k=7;k>=0;--k){
             x = (content & (1 << k));  //Write to Address
             status_reg <<= 1;
             Send_Bit(x);

             if ((P1IN & MISO) == 0x02){


                                             status_reg |= 0b00000001;
                                                }
                                                else {


                                             status_reg  &= 0b11111110;
                                                }

             SCLK_Pulse();

                         }

}
void Read_Byte_MSB_First(int index, unsigned char regname[])
{
    for (i=0;i<=(index-1);i++){
        for (k=0;k<8;k++){
           regname[i] <<= 1;


                     if ((P1IN & MISO) == 0x02){

                                                 //read_reg |= 0b10000000;
                                                   regname[i] |= 0b00000001;
                                                    }
                                                    else {

                                                 //read_reg  &= 0b01111111;
                                                   regname[i]  &= 0b11111110;
                                                    }
                     SCLK_Pulse();
    }
}
}
void Write_Byte_MSB_First(unsigned char content[], int index2)
{
    for (i=0;i<=(index2-1);i++){
    for (k=7;k>=0;--k){

             x = (content[i] & (1 << k));  //Write to Address
             Send_Bit(x);
             SCLK_Pulse();

                         }

}
}

void Write_Payload_MSB_First(int pyld[], int index3)
{
    for (pd_i=0;pd_i<=(index3-1);pd_i++){
        for (pd=7;pd>=0;--pd){

                     pd_x = (pyld[pd_i] & (1 << pd));  //Write to Address
                     Send_Bit(pd_x);
                     SCLK_Pulse();

                                 }
        for (pd=15;pd>=8;--pd){

                             pd_x = (pyld[pd_i] & (1 << pd));  //Write to Address
                             Send_Bit(pd_x);
                             SCLK_Pulse();

                                         }

        }
}


void motorCW(void)
{
    //CLOCKWISE MOTOR ROTATION
//delay >1950
    //1,2
    __delay_cycles(3000);
    P2OUT &= ~COIL3 + ~COIL4;//coil 3 and 4 off
    P1OUT |= COIL1; //1,2 on
    P2OUT |= COIL2;
    //2,3
    __delay_cycles(3000);
    P2OUT &= ~COIL4; // coil 4 off
    P1OUT &= ~COIL1; //coil 1 off
    P2OUT |= COIL2 + COIL3; // coil 3 on and coil 2 on

    //3,4
    __delay_cycles(3000);
    P2OUT |=COIL3 + COIL4 ; //coil 3 and 4 on
    P2OUT &= ~COIL2; //coil 2 off

    P1OUT &= ~COIL1; // coil 1 and 2 off

    //4,1
    __delay_cycles(3000);
    P2OUT &= ~COIL2;
    P2OUT &= ~COIL3; //coil 3 off and coil 2 off
    P2OUT |= COIL4; //coil 4 on
    P1OUT |= COIL1; //coil 1 on
    __delay_cycles(3000);


}
void motorCCW(void)
{
    //CLOCKWISE MOTOR ROTATION
//delay >1950
    //4,1
    __delay_cycles(3000);
    P2OUT &= ~COIL2;
    P2OUT &= ~COIL3; //coil 3 off and coil 2 off
    P2OUT |= COIL4; //coil 4 on
    P1OUT |= COIL1; //coil 1 on
    __delay_cycles(3000);

    //3,4
    __delay_cycles(3000);
    P2OUT |=COIL3 + COIL4 ; //coil 3 and 4 on
    P2OUT &= ~COIL2; //coil 2 off

    P1OUT &= ~COIL1; // coil 1 and 2 off

    //2,3
    __delay_cycles(3000);
    P2OUT &= ~COIL4; // coil 4 off
    P1OUT &= ~COIL1; //coil 1 off
    P2OUT |= COIL2 + COIL3; // coil 3 on and coil 2 on

    //1,2
    __delay_cycles(3000);
    P2OUT &= ~COIL3 + ~COIL4;//coil 3 and 4 off
    P1OUT |= COIL1; //1,2 on
    P2OUT |= COIL2;



}
void motorStop(void)
{
    P2OUT &= ~COIL2; //coil 3 off //coil 2 off //coil 4 off
    P2OUT &=  ~COIL3;
    P2OUT &=  ~COIL4;
    P1OUT &= ~COIL1; //coil 1 off

}
