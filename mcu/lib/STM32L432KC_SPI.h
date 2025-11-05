// STM32L432KC_SPI.h
// Eoin O'Connell
// eoconnell@hmc.edu
// Fri Oct. 17 2025
// Header file for SPI protocol

#ifndef STM32L4_SPI_H
#define STM32L4_SPI_H

#include <stdint.h>
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Pin Definitions
///////////////////////////////////////////////////////////////////////////////

#define SPI_NSS PA4
#define SPI_SCK PA5 
#define SPI_MISO PA6 // SDI
#define SPI_MOSI PA7 // SDO

//#define SPI_SCK PB3
//#define SPI_NSS PA5
//#define SPI_MOSI PB5 // SDO
//#define SPI_MISO PB4 // SDI


///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Enables the SPI peripheral and intializes its clock speed (baud rate), polarity, and phase.
 *    -- br: (0b000 - 0b111). The SPI clk will be the master clock / 2^(BR+1).
 *    -- cpol: clock polarity (0: inactive state is logical 0, 1: inactive state is logical 1).
 *    -- cpha: clock phase (0: data captured on leading edge of clk and changed on next edge, 
 *          1: data changed on leading edge of clk and captured on next edge)
 * Refer to the datasheet for more low-level details. */ 
void initSPI(int br, int cpol, int cpha);

/* Transmits a character (1 byte) over SPI and returns the received character.
 *    -- send: the character to send over SPI
 *    -- return: the character received over SPI */
char spiSendReceive(char send);

#endif