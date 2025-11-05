#include "stm32l4xx.h"

// === DS1722 Definitions ===
#define DS1722_CS_PORT     GPIOA
#define DS1722_CS_PIN      GPIO_PIN_4

#define DS1722_READ_TEMP_MSB_CMD   0x01
#define DS1722_DUMMY_BYTE          0x00

// === SPI & GPIO Initialization ===
void SPI1_GPIO_Init(void) {
    // Enable GPIOA clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // PA5 = SCK, PA6 = MISO, PA7 = MOSI (AF5)
    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOA->MODER |= (GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1); // Alternate function
    GPIOA->AFR[0] |= (5 << (5 * 4)) | (5 << (6 * 4)) | (5 << (7 * 4)); // AF5

    // PA4 = CS as GPIO output
    GPIOA->MODER &= ~GPIO_MODER_MODE4;
    GPIOA->MODER |= GPIO_MODER_MODE4_0; // Output mode
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT4; // Push-pull
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4; // High speed
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;

    // Set CS high (inactive)
    DS1722_CS_PORT->ODR |= DS1722_CS_PIN;
}

void SPI1_Init(void) {
    // Enable SPI1 clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Disable SPI for configuration
    SPI1->CR1 &= ~SPI_CR1_SPE;

    // Configure SPI1
    SPI1->CR1 = 0;
    SPI1->CR1 |= SPI_CR1_MSTR;             // Master mode
    SPI1->CR1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0; // Baud rate = fPCLK/8
    SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA; // Mode 3: CPOL=1, CPHA=1
    SPI1->CR1 |= SPI_CR1_SSI | SPI_CR1_SSM;   // Software slave management

    SPI1->CR2 = 0; // 8-bit data, default

    // Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

// === Helper Functions ===
static void DS1722_Select(void) {
    DS1722_CS_PORT->BSRR = (uint32_t)DS1722_CS_PIN << 16; // CS Low
}

static void DS1722_Deselect(void) {
    DS1722_CS_PORT->BSRR = DS1722_CS_PIN; // CS High
}

static uint8_t SPI1_TransmitReceive(uint8_t data) {
    while (!(SPI1->SR & SPI_SR_TXE));               // Wait until TX buffer is empty
    *((__IO uint8_t *)&SPI1->DR) = data;            // Send data
    while (!(SPI1->SR & SPI_SR_RXNE));              // Wait until RX buffer full
    return *((__IO uint8_t *)&SPI1->DR);            // Return received byte
}

// === Temperature Read Function ===
float DS1722_ReadTemperature(void) {
    uint8_t msb, lsb;
    int16_t rawTemp;
    float temperature;

    DS1722_Select();

    SPI1_TransmitReceive(DS1722_READ_TEMP_MSB_CMD); // Send read temp command
    msb = SPI1_TransmitReceive(DS1722_DUMMY_BYTE);  // Read MSB
    lsb = SPI1_TransmitReceive(DS1722_DUMMY_BYTE);  // Read LSB

    DS1722_Deselect();

    rawTemp = ((msb << 8) | lsb) >> 4; // 12-bit signed
    if (rawTemp & 0x800) {             // Negative value, sign-extend
        rawTemp |= 0xF000;
    }

    temperature = rawTemp * 0.0625f;
    return temperature;
}

// === Main Example (Optional for Test) ===
int main(void) {
    SPI1_GPIO_Init();
    SPI1_Init();

    while (1) {
        float tempC = DS1722_ReadTemperature();
        // Use temperature (e.g., send over UART, store, display, etc.)
        for (volatile int i = 0; i < 1000000; ++i); // Simple delay
    }
}
