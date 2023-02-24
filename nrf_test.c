/*
 *  ======= nrf_test ========
 *  nrf_test target-side implementation
 *
 *  Created on: Feb. 5, 2023
 *  Author:     abhis
 */
#include <msp430.h>
#include <stdint.h>
#include <string.h>

// Pin definitions
#define CE_PIN BIT0
#define CSN_PIN BIT1
#define STEP_1_PIN BIT2
#define STEP_2_PIN BIT3
#define STEP_3_PIN BIT4
#define STEP_4_PIN BIT5

// Motor control pins
const uint8_t stepPins[] = {STEP_1_PIN, STEP_2_PIN, STEP_3_PIN, STEP_4_PIN};

// Stepper motor variables
const uint8_t stepCount = 8;
const uint16_t stepsPerRevolution = 512;
const uint16_t stepDelay = 500;

// Command buffer
char cmdBuffer[32];
uint8_t cmdIndex = 0;

void initGPIO(void) {
  // Configure CE and CSN pins for NRF24L01
  P1DIR |= CE_PIN | CSN_PIN;
  P1OUT &= ~CE_PIN;
  P1OUT |= CSN_PIN;

  // Configure stepper motor pins
  P1DIR |= STEP_1_PIN | STEP_2_PIN | STEP_3_PIN | STEP_4_PIN;
  P1OUT &= ~STEP_1_PIN & ~STEP_2_PIN & ~STEP_3_PIN & ~STEP_4_PIN;
}

void initSPI(void) {
  // Configure USCI_A0 for SPI communication with NRF24L01
  UCB0CTL1 |= UCSWRST;
  UCB0CTL0 |= UCMST | UCSYNC | UCCKPH;
  UCB0CTL1 |= UCSSEL_2;
  UCB0BR0 = 0x02;
  UCB0BR1 = 0x00;
  UCB0CTL1 &= ~UCSWRST;
}

void sendSPI(uint8_t data) {
  UCB0TXBUF = data;
  while (!(UCB0IFG & UCTXIFG)) {};
}

uint8_t receiveSPI(void) {
  while (!(UCB0IFG & UCRXIFG)) {};
  return UCB0RXBUF;
}

void selectNRF(void) {
  P1OUT &= ~CSN_PIN;
}

void deselectNRF(void) {
  P1OUT |= CSN_PIN;
}

void powerUpNRF(void) {
  P1OUT |= CE_PIN;
  __delay_cycles(10);
}

void powerDownNRF(void) {
  P1OUT &= ~CE_PIN;
}

void writeNRFRegister(uint8_t reg, uint8_t value) {
  selectNRF();
  sendSPI(W_REGISTER | (REGISTER_MASK & reg));
  sendSPI(value);
  deselectNRF();
  }

  uint8_t readNRFRegister(uint8_t reg) {
    selectNRF();
    sendSPI(R_REGISTER | (REGISTER_MASK & reg));
    uint8_t value = receiveSPI();
    deselectNRF();
    return value;
  }

  void writeNRFPayload(char *data, uint8_t length) {
    selectNRF();
    sendSPI(W_TX_PAYLOAD);
    for (uint8_t i = 0; i < length; i++) {
      sendSPI(data[i]);
    }
    deselectNRF();
  }

  void readNRFPayload(char *data, uint8_t length) {
    selectNRF();
    sendSPI(R_RX_PAYLOAD);
    for (uint8_t i = 0; i < length; i++) {
      data[i] = receiveSPI();
    }
    deselectNRF();
  }

  void stepMotor(uint8_t step) {
    for (uint8_t i = 0; i < 4; i++) {
      if (step & (1 << i)) {
        P1OUT |= stepPins[i];
      } else {
        P1OUT &= ~stepPins[i];
      }
    }
  }

  void rotateMotor(int16_t steps) {
    uint16_t stepIndex = 0;
    uint16_t stepDelayMs = (1000 * stepDelay) / stepsPerRevolution;
    int16_t stepDirection = steps > 0 ? 1 : -1;
    steps = abs(steps);

    for (uint16_t i = 0; i < steps; i++) {
      stepMotor(stepIndex % stepCount);
      stepIndex += stepDirection;
      __delay_cycles(stepDelayMs * 1000);
    }

    stepMotor(0);
  }

  void processCommand(char *cmd) {
    int16_t steps = 0;

    if (strcmp(cmd, "rotate_left") == 0) {
      steps = -stepsPerRevolution;
    } else if (strcmp(cmd, "rotate_right") == 0) {
      steps = stepsPerRevolution;
    } else {
      return;
    }

    rotateMotor(steps);
  }

  int main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    initGPIO();
    initSPI();

    powerUpNRF();

    // Configure NRF24L01 for receiving data
    // ...

    while (1) {
      // Read data from NRF24L01
        uint8_t status = readNRFStatus();
        if (status & (1 << RX_DR)) {
          char data[32];
          readNRFPayload(data, 32);
          processCommand(data);
        }
      }
    }

