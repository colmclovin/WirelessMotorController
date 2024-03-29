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

//This code, together with the nrf24.h header file configures the nRF24L01+ as a RECEIVER
//and receives 8 integer values from the transmitter (Arduino nano & nrf24L01+ & mq gas sensors - which are not necessary,
//any 8 integer values will be OK. And it displays the values on serial input -19200 baud, I'll use Putty this time
//Please check my YouTube channel & subscribe for the related content:
//https://www.youtube.com/c/drselim

#define MOSI BIT2  //   P1.2
#define MISO BIT1  //   P1.1
#define SCLK BIT4  //   P1.4
#define CE BIT7    //   P1.7
#define CSN BIT3   //   P1.3
int i;
int j;
int k;
int l; //for pload2, so many indexes..
int x;
int pd; //payload func index 0-15
int pd_i;  //payload func i index 0-7
int pd_x;
int pipe_nr;
int pyld1[8]; //1st 16 bytes in rx fifo
int pyld2[8]; //2nd 16 bytes in rx fifo

unsigned char status_reg;
unsigned char read_reg[5];
char buf[5];
char pipe_nr_chr[5];
char bosluk[] = " ";
char next_satir[] = "\r\n";  //sorry for mixing eng and tur. i'm doing it on purpose :)

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
void ser_output(char *str);  //Serial output func

void main(void)
    {

    __delay_cycles(100); //power on reset

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    //P2DIR &= 0x00 ;
    P1OUT &= 0x00;
    P1DIR |= MOSI + SCLK + CE + CSN ;  //Output Pins
    P1DIR &= ~MISO;
    /*****************************************
    **SETTINGS FOR SERIAL MONITOR 19200 baud**
    *****************************************/
    P2SEL = BIT1|BIT2; //fix
    P2SEL2 = BIT1|BIT2; //fix
    UCA0CTL1 |= UCSWRST+UCSSEL_2;
    UCA0BR0 = 52;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_0;
    UCA0CTL1 &= ~UCSWRST;
    /***********************************
    ** END SETTINGS FOR SERIAL MONITOR**
    ************************************/
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
            ltoa(pipe_nr,pipe_nr_chr,8);
            //ser_output(pipe_nr_chr);
            //ser_output(next_satir);
            j=0;
            l=0;
            for (i=0;i<=14;i+=2){
                pyld1[j]=read_PAYLOAD[i] | (read_PAYLOAD[i+1] << 8);
                ltoa(pyld1[j],buf,8);
                ser_output(buf); ser_output(bosluk);
                j++;

            }
            ser_output(next_satir);
            for (i=16;i<=30;i+=2){
                            pyld2[l]=read_PAYLOAD[i] | (read_PAYLOAD[i+1] << 8);
                            ltoa(pyld1[j],buf,8);
                            //ser_output(buf); ser_output(bosluk);
                            l++;
                        }
            //ser_output(next_satir);
            CSN_Off();
            Instruction_Byte_MSB_First(W_REGISTER | STATUS);
            Write_Byte_MSB_First(clr_status,1);
            CSN_On();
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
void ser_output(char *str){
    while(*str != 0){
        while (!(IFG2&UCA0TXIFG));
        UCA0TXBUF = *str++;
    }
}
