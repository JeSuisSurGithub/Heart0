#include "dac.h"
#include "midi.h"
#include "waves.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


volatile uint16_t audio_sample= MID_VALUE;
volatile uint16_t phase = 0;

volatile uint8_t g_wave_select = 0;
volatile uint16_t g_pwm = MID_VALUE;

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

    // Timer1
    // Normal mode
    // CTC, prescaler=1
    // 32KHz, compare match interrupt
    // Enable interrupt
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10);
    OCR1A = (F_CPU / SAMPLE_RATE) - 1;
    TIMSK1 = (1 << OCIE1A);
}

uint16_t pwm_saw(uint16_t phase, uint16_t pw)
{
    uint16_t saw = phase >> 4;   // 0..4095

    uint16_t half_pw = pw >> 1;

    if (saw > (MID_VALUE - half_pw) && saw < (MID_VALUE + half_pw)) {
        return 0;   // absolute minimum
    }
    else {
        return saw;
    }
}

uint16_t pwm_squ(uint16_t phase, uint16_t pw)
{
    uint16_t squ = phase >> 4;   // 0..4095

    if (squ > pw) {
        return 0;
    }
    else {
        return 4095;
    }
}

uint16_t read_wave(uint16_t phase, const uint8_t wave_table[256])
{
    uint16_t value = pgm_read_byte(&wave_table[phase >> 8]) << 8;
    return value >> 4;
}

ISR(TIMER1_COMPA_vect)
{
    phase += g_phase_inc;

    switch (g_wave_select) {
        case PW_SAW: audio_sample = pwm_saw(phase, g_pwm >> 4); break;
        case PW_SQU: audio_sample = pwm_squ(phase, g_pwm >> 4); break;
        case SAW: audio_sample = pwm_saw(phase, 0); break;
        case SQU: audio_sample = pwm_squ(phase, MID_VALUE); break;
        case SINE: audio_sample = read_wave(phase, sine_table); break;
        case CAMEL: audio_sample = read_wave(phase, camel_sine_table); break;
        case TRI: audio_sample = read_wave(phase, tri_table); break;
        case TRI_SAW: audio_sample = read_wave(phase, tri_saw_table); break;
        case ABS: audio_sample = read_wave(phase, abs_sine_table); break;
        case HALF: audio_sample = read_wave(phase, half_sine_table); break;
        default: audio_sample = MID_VALUE; break;
    }

    dac_write(WAVE, audio_sample);
}