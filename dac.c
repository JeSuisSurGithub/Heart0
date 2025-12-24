#include "dac.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile bool g_update_dac_flag = false;

void spi_send(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
}

void dac_write(DAC_CH channel, uint16_t value) {
    PORTB &= ~(1 << PORT_CS);

    switch (channel)
    {
        case WAVE:
            spi_send(0x30 | ((value >> 8) & 0x0F));
            break;
        case FILTER_CV:
            spi_send(0xB0 | ((value >> 8) & 0x0F));
            break;
        default: break;
    }

    spi_send(value & 0xFF);

    PORTB |= (1 << PORT_CS);
}

void init_dac()
{
    // Enable CS MOSI SCK
    // Enable SPI in Master mode
    // 8MHz -> 4MHz speed, least divider
    DDRB |= (1 << PORT_CS) | (1 << PORT_MOSI) | (1 << PORT_SCLK);
    SPCR = (1 << SPE) | (1 << MSTR) | (0 << SPR1) | (0 << SPR0);
    SPSR = (1 << SPI2X);
    PORTB |= (1 << PORT_CS);
}