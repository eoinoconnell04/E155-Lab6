// DS1722.h
// Eoin O'Connell
// eoconnell@hmc.edu
// Fri Oct. 17 2025
// All functions for DS1722 temp sensor

#include "DS1722.h"
#include "STM32L432KC.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_SPI.h"
#include "STM32L432KC_TIM.h"
#include <stdint.h>
#include <stdbool.h>

// Initialization Function for temperature sensor (also intializes SPI)
void initDS1722() {
    // Initialize SPI
    initSPI(0b111, 0, 1);
  
    // set chip select
    digitalWrite(SPI_NSS, PIO_HIGH);

    // write to config register
    spiSendReceive(0x80); // Configuration Register Address
    spiSendReceive(0xEE); 

    // unset chip select (transaction over)
    digitalWrite(SPI_NSS, PIO_LOW);
}

// Function that takes input of the desired number of bits of precision and sets it to the temperature sensor config register
// Note, waits in function to make sure that the new convertion has finished before next reading
void setPrecision(int digits) {
    digitalWrite(SPI_NSS, PIO_HIGH);
    spiSendReceive(0x80);
    switch (digits) {
        case 8: spiSendReceive(0xE0); break; // delay_millis(TIM15, 75); 
        case 9: spiSendReceive(0xE2); break;   // delay_millis(TIM15, 150); 
        case 10: spiSendReceive(0xE4); break; //  delay_millis(TIM15, 300);
        case 11: spiSendReceive(0xE6); break; //  delay_millis(TIM15, 600);
        case 12: spiSendReceive(0xE8); break; //  delay_millis(TIM15, 700);
        default: break;
    }
    digitalWrite(SPI_NSS, PIO_LOW);

    readTemp();
}

// Function that takes no inputs and returns the current temperature 
float readTemp() {

    // MSB Transaction
    digitalWrite(SPI_NSS, PIO_HIGH);
    spiSendReceive(0x02); // read address for msb
    uint8_t msb = spiSendReceive(0x00);
    digitalWrite(SPI_NSS, PIO_LOW);

    bool sign = (msb & 0x80) != 0;  // extract bit 7
    uint8_t mag = msb & 0x7F; // extract bits 0-6

    // LSB Transaction
    digitalWrite(SPI_NSS, PIO_HIGH);
    spiSendReceive(0x01); // read address for lsb
    uint8_t lsb = spiSendReceive(0x00);
    digitalWrite(SPI_NSS, PIO_LOW);

    float temp = (1 - 2 * (int)sign) * (mag + (lsb / 256.0f));

    return temp;
}