#include <msp430.h>
#include <stdint.h>
#include <string.h>
#include "nrf24l01.h"
#include <stdio.h>
#include <RF24.h>
#include "RF24_config.h"


#define MOSI BIT2
#define MISO BIT1
#define SCK BIT4
#define CE BIT7
#define CSN BIT5

void spi_init(void)
{
    UCB0CTL1 |= UCSWRST;                    // Put USCI state machine in reset
    UCB0CTL0 = UCCKPH + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;                   // SMCLK as clock source
    UCB0BR0 = 0x02;                         // SMCLK / 2
    UCB0BR1 = 0;
    P1SEL |= MOSI + MISO + SCK;             // P1.2, P1.1, P1.4 USCI_B0 option select
    P1SEL2 |= MOSI + MISO + SCK;            // P1.2, P1.1, P1.4 USCI_B0 option select
    UCB0CTL1 &= ~UCSWRST;                   // Initialize USCI state machine
}

void nrf_ce_high(void)
{
    P1OUT |= CE;
}

void nrf_ce_low(void)
{
    P1OUT &= ~CE;
}

void nrf_csn_high(void)
{
    P1OUT |= CSN;
}

void nrf_csn_low(void)
{
    P1OUT &= ~CSN;
}

void nrf_init(void)
{
    spi_init();
    P1DIR |= CE + CSN;                      // CE, CSN as outputs
    nrf_ce_low();
    nrf_csn_high();
    nrf_write_register(NRF_REG_CONFIG, 0x08);  // Enable 1-byte CRC, PTX mode
}

void nrf_send_data(uint8_t *data, uint8_t len)
{
    nrf_ce_low();
    nrf_csn_low();
    spi_transfer_byte(NRF_CMD_W_TX_PAYLOAD);
    uint8_t i;
    for (i = 0; i < len; i++)
    {
        spi_transfer_byte(data[i]);
    }

    nrf_csn_high();
    nrf_ce_high();
    __delay_cycles(10);
    nrf_ce_low();
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;               // Stop WDT
    nrf_init();                             // Initialize NRF24L01

    while (1)
    {
        uint8_t data[] = "Hello, Arduino!"; // Data to be sent
        uint8_t len = strlen((char*)data);  // Length of data
        nrf_send_data(data, len);           // Send data
        __delay_cycles(1000000);            // Wait 1 second
    }
}
