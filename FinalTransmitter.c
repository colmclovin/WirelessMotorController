/*
  Copyright (C) 2022 Selim Gullulu <drselim2021@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.
  */

#include <msp430.h>
#include "nrf24l01.h"

//This code, together with the nrf24.h header file configures the nRF24L01+ as a Transmitter
//and sends 8 integer values to a receiver (Arduino nano & nrf24L01+) to be displayed on serial monitor.
//Please check my YouTube channel & subscribe for the related content:
//https://www.youtube.com/c/drselim

#define MOSI BIT2  //   P1.2
#define MISO BIT1  //   P1.1
#define SCLK BIT4  //   P1.4
#define CE BIT7    //   P1.7
#define CSN BIT5   //   P1.5 for transmitter and P1.3 for reciever
#define ACTIVATE BIT6
//#define LED BIT0

#define OPEN BIT0
#define CLOSE BIT3


#define PWR_LED BIT3
#define LED1 BIT1
#define LED2 BIT2
#define LED3 BIT0
#define LED4 BIT4
#define LED5 BIT5



int i;
int j;
int k;
int x;
int pd; //payload func index 0-15
int pd_i;  //payload func i index 0-7
int pd_x;

unsigned char status_reg;
unsigned char read_reg[5];

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

unsigned char clr_status[1]={0x70};

unsigned char rf_setupregister[1]={0b00000001};
unsigned char configregister[1]={0b00001110};
unsigned char rf_chanregister[1]={0b01001100};
unsigned char address[6]="00001";
unsigned char setup_retr_register[1]={0b01011111};
unsigned char en_aa_register[1]={0b00111111};
unsigned char rx_pw_register[1]={0b00100000};

void SCLK_Pulse (void);
void Send_Bit (unsigned int value);
void CE_On (void);  //Chip enable
void CE_Off (void);  //Chip disable
void CSN_On (void);     //CSN On
void CSN_Off (void);    //CSN Off
void Write_Byte (int content);
void Instruction_Byte_MSB_First (int content);
void Read_Byte_MSB_First(int index, unsigned char regname[]);
void Write_Byte_MSB_First(unsigned char content[], int index2);
void Write_Payload_MSB_First(int pyld[], int index3);
//void Init_Registers(void);

void main(void)
    {
    //Technically done the code, need to test the buttons
    // Will need to update to work with the 4Mhz oscillator

    int payloadClose[8] = {0,0,0,0,0,0,0,0};
    int payloadOpen[8] = {1,1,1,1,1,1,1,1};

    __delay_cycles(100); //power on reset

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //LED DECLARATION
    P2DIR |= LED3 + LED1 + LED2 + PWR_LED + LED4 + LED5;
    P2OUT |= PWR_LED;
    P2OUT &= ~LED4 + ~LED5;

    P1OUT &= 0x00;
    P1DIR |= MOSI + SCLK + CE + CSN;  //Output Pins //11111111
    P1DIR &= ~MISO + ~ACTIVATE; // +~OPEN + ~CLOSE; //11111110
    P1REN |= ACTIVATE + OPEN + CLOSE;              //Pull Up/Down Enable
    P1OUT |= ACTIVATE + OPEN + CLOSE;              //Pull Up Enable

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
            __delay_cycles(2000);  //start_up 1.5 ms


//    Init_Registers(); //Initializes the nRF module registers


    while(1){
        //put into sleep mode until button is pressed maybe :?


        if(P1IN & ACTIVATE)
        {// If SW is NOT pressed
            //DO NOTHING

        }
                else
                { //ACTIVATE SW is pressed

                    if(P1IN & CLOSE) //CLOSE PIN IS SELECTED
                    {

                    }
                    else
                    {
                        P2OUT |= LED4; // LED4 ON
                        //STDBY-I
                        int z;
                            for(z=0; z<10;z++)
                            {
                                CSN_Off();
                                Instruction_Byte_MSB_First(W_TX_PAYLOAD);
                                Write_Payload_MSB_First(payloadClose,8);
                                Write_Payload_MSB_First(payloadClose,8);
                                CSN_On();
                                CE_On();
                                __delay_cycles(50); //min pulse >10usec
                                CE_Off();
                                //TX settling 130 usec
                                __delay_cycles(150);
                                //TX MODE

                                __delay_cycles(20000);
                                //STDBY-I
                                CSN_Off();
                                Instruction_Byte_MSB_First(NOP);
                                CSN_On();
                                if ((status_reg & BIT4) == 0x10){
                                        CSN_Off();
                                        Instruction_Byte_MSB_First(W_REGISTER | STATUS);
                                        Write_Byte_MSB_First(clr_status,1);
                                        CSN_On();
                                        CSN_Off();
                                        Instruction_Byte_MSB_First(FLUSH_TX);
                                        CSN_On();
                                        __delay_cycles(10000);
                                }

                                }
                    }
                    if(P1IN & OPEN) //OPEN PIN SELECTED
                    {
                        //do nothing
                    }
                    else
                    {
                        P2OUT |= LED5;// else LED ON
                        int b;
                        for(b=0; b<10;b++)
                        {

                                            CSN_Off();
                                            Instruction_Byte_MSB_First(W_TX_PAYLOAD);
                                            Write_Payload_MSB_First(payloadOpen,8);
                                            Write_Payload_MSB_First(payloadOpen,8);
                                            CSN_On();
                                            CE_On();
                                            __delay_cycles(50); //min pulse >10usec
                                            CE_Off();
                                            //TX settling 130 usec
                                            __delay_cycles(150);
                                            //TX MODE

                                            __delay_cycles(20000);
                                            //STDBY-I
                                            CSN_Off();
                                            Instruction_Byte_MSB_First(NOP);
                                            CSN_On();
                                            if ((status_reg & BIT4) == 0x10){
                                                    CSN_Off();
                                                    Instruction_Byte_MSB_First(W_REGISTER | STATUS);
                                                    Write_Byte_MSB_First(clr_status,1);
                                                    CSN_On();
                                                    CSN_Off();
                                                    Instruction_Byte_MSB_First(FLUSH_TX);
                                                    CSN_On();
                                                    __delay_cycles(10000);
                                            }

                    }
                    }


                    } //ACTIVATE ELSE

        P2OUT &= ~LED4 + ~LED5;
                    }//WHILE(1)


                }//INT MAIN

//void Init_Registers(void)
//{
//
//}
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
void Write_Byte(int content)  //Not ued in this application
{

    for (j=0;j<8;j++){
             x = (content & (1 << j));  //Write to Address
             Send_Bit(x);
             SCLK_Pulse();
        }
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

