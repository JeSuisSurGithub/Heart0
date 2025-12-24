#include "common.h"
#include "midi.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

const uint16_t phase_lut[128] PROGMEM = {
    32, 34, 36, 39, 41, 43, 46, 49,
    52, 55, 58, 62, 65, 69, 73, 78,
    82, 87, 92, 98, 104, 110, 117, 124,
    131, 139, 147, 156, 165, 175, 185, 197,
    208, 221, 234, 248, 263, 278, 295, 312,
    331, 351, 371, 394, 417, 442, 468, 496,
    526, 557, 590, 625, 662, 702, 743, 788,
    835, 884, 937, 993, 1052, 1114, 1180, 1251,
    1325, 1404, 1487, 1576, 1670, 1769, 1874, 1986,
    2104, 2229, 2361, 2502, 2651, 2808, 2975, 3152,
    3340, 3538, 3749, 3972, 4208, 4458, 4723, 5004,
    5302, 5617, 5951, 6305, 6680, 7077, 7498, 7944,
    8417, 8917, 9447, 10009, 10604, 11235, 11903, 12611,
    13361, 14155, 14997, 15889, 16834, 17835, 18895, 20019,
    21209, 22470, 23807, 25222, 26722, 28311, 29995, 31778,
    33668, 35670, 37791, 40038, 42419, 44941, 47614, 50445
};

volatile uint8_t midi_status = 0;
volatile uint8_t midi_data1 = 0;
volatile uint8_t midi_byte_count = 0;
volatile uint8_t last_note = 0;
volatile uint8_t g_note_on_flag = 0;
volatile uint16_t g_phase_inc = 0;
volatile bool g_update_keypress = false;

void init_midi()
{
    // Vgate, 31.25KHz MIDI
    // Normal speed, enable RX and ISR, 8N1
    DDRD &= ~(1 << PORT_RX);
    DDRB |= (1 << PORT_VGATE);
    uint16_t ubrr = (F_CPU / (16UL * 31250)) - 1;
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr & 0xFF;

    UCSR0A = 0;
    UCSR0B = (1<<RXEN0) | (1<<RXCIE0);
    UCSR0C = (0<<USBS0) | (0<<UPM01) | (0<<UPM00) | (1<<UCSZ01) | (1<<UCSZ00);
}

ISR(USART_RX_vect) {
    uint8_t b = UDR0;

    // Status byte
    if (b & 0x80) {
        midi_status = b;
        midi_byte_count = 0;
        return;
    }

    switch (midi_status & 0xF0) {

        case 0x90: // Note ON
            if (midi_byte_count == 0) {
                midi_data1 = b;
                midi_byte_count = 1;
            } else {
                if (b == 0) {
                    g_note_on_flag = 0;
                } else {
                    last_note = midi_data1;
                    g_note_on_flag = 1;
                    g_phase_inc = pgm_read_word(&phase_lut[last_note]);
                    g_update_keypress = true;
                }
                midi_byte_count = 0;
            }
            break;

        case 0x80: // Note OFF
            if (midi_byte_count == 0) {
                midi_data1 = b;
                midi_byte_count = 1;
            } else {
                g_note_on_flag = 0;
                // phase_inc = 0;
                midi_byte_count = 0;
            }
            break;
    }

    if (g_note_on_flag) {
        PORTB |= (1 << PORT_VGATE);   // PB1 HIGH
    } else {
        PORTB &= ~(1 << PORT_VGATE);  // PB1 LOW
    }
}