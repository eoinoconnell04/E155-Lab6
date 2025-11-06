// STM32L432KC_SPI.c
// Eoin O'Connell
// eoconnell@hmc.edu
// Fri Oct. 17 2025
// File containing all functions for SPI protocol

// Include header files:
#include "STM32L432KC.h"

// Initialize SPI function
// Inputs: integers bod rate, polarity, and clock phase
void initSPI(int br, int cpol, int cpha) {
    
    // enable GPIO 
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // Initialize clock for SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // enable SPI1 clock 
    
    // Configure GPIO pins for MOSI, MISO, SCK pins
    pinMode(SPI_MISO, GPIO_ALT); // configure MISO pin
    pinMode(SPI_MOSI, GPIO_ALT); // confgirue MOSI pin
    pinMode(SPI_SCK, GPIO_ALT); // configure SCK pin   
    pinMode(SPI_NSS, GPIO_OUTPUT); // configure NSS pin as manual
    digitalWrite(SPI_NSS, PIO_LOW); // disable SPI connection

    GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED3; // make PA5 high speed

    // set pins to alternate function 5
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 5);
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL6, 5);
    GPIOA->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL7, 5);

    // Write to SPI_CR1 Reg
    SPI1->CR1 = 0; // reset SPI_CR1 to default state
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br); // set bod rate
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol); // set polarity bit
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha); // set phase bit
    SPI1->CR1 |= SPI_CR1_MSTR; // set master mode

    // Write to SPI_CR2 Reg
    SPI1->CR2 = 0; // reset SPI_CR2 to default state
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111); // set package length to 8 bits
    SPI1->CR2 |= SPI_CR2_FRXTH; // set FRXTH to 1
    SPI1->CR2 |= SPI_CR2_SSOE; // set SSOE to 1

    SPI1->CR1 |= SPI_CR1_SPE; // enable SPI
}

// Function to send and recieve signals via SPI
// Input char: address or data
// Output char: data
// Usage: 
//      read:  call once to send address, call second time with dummy variable to read data
//      write: call once to send address, call second time to send data
char spiSendReceive(char send) {
    //while((SPI1->SR & SPI_SR_TXE) == 0); // Wait until the transmit buffer is empty
    while(!(SPI1->SR & SPI_SR_TXE)); // Wait until the transmit buffer is empty

    *(volatile char *) (&SPI1->DR) = send; // Transmit the character over SPI
    while(!(SPI1->SR & SPI_SR_RXNE)); // Wait until data has been received
    //while((SPI1->SR & SPI_SR_RXNE) == 0); // Wait until data has been received
    char rec = (volatile char) SPI1->DR; // Capture return transmission

    return rec; // Return received character
}



