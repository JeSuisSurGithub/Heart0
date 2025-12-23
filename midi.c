#include "common.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

const uint32_t phase_lut[128] PROGMEM = {
    1077385UL, 1141450UL, 1209324UL, 1281234UL, 1357420UL, 1438137UL, 1523653UL, 1614254UL,
    1710242UL, 1811939UL, 1919682UL, 2033833UL, 2154771UL, 2282900UL, 2418648UL, 2562469UL,
    2714841UL, 2876274UL, 3047306UL, 3228508UL, 3420485UL, 3623878UL, 3839365UL, 4067666UL,
    4309542UL, 4565801UL, 4837297UL, 5124938UL, 5429683UL, 5752548UL, 6094613UL, 6457017UL,
    6840971UL, 7247757UL, 7678731UL, 8135332UL, 8619084UL, 9131602UL, 9674595UL, 10249876UL,
    10859366UL, 11505097UL, 12189226UL, 12914035UL, 13681943UL, 14495514UL, 15357462UL, 16270665UL,
    17238169UL, 18263204UL, 19349190UL, 20499753UL, 21718732UL, 23010195UL, 24378452UL, 25828070UL,
    27363887UL, 28991029UL, 30714925UL, 32541330UL, 34476338UL, 36526408UL, 38698381UL, 40999506UL,
    43437464UL, 46020390UL, 48756905UL, 51656141UL, 54727775UL, 57982058UL, 61429851UL, 65082660UL,
    68952676UL, 73052816UL, 77396762UL, 81999013UL, 86874928UL, 92040780UL, 97513810UL, 103312283UL,
    109455551UL, 115964116UL, 122859702UL, 130165320UL, 137905353UL, 146105632UL, 154793525UL, 163998026UL,
    173749857UL, 184081561UL, 195027620UL, 206624566UL, 218911102UL, 231928233UL, 245719404UL, 260330640UL,
    275810706UL, 292211264UL, 309587050UL, 327996053UL, 347499714UL, 368163122UL, 390055241UL, 413249132UL,
    437822204UL, 463856467UL, 491438808UL, 520661281UL, 551621412UL, 584422528UL, 619174100UL, 655992107UL,
    694999428UL, 736326245UL, 780110482UL, 826498265UL, 875644409UL, 927712935UL, 982877617UL, 1041322562UL,
    1103242824UL, 1168845056UL, 1238348200UL, 1311984215UL, 1389998857UL, 1472652490UL, 1560220964UL, 1652996530UL
};

volatile uint8_t midi_status = 0;
volatile uint8_t midi_data1 = 0;
volatile uint8_t midi_byte_count = 0;
volatile uint8_t last_note = 0;
volatile uint8_t g_note_on_flag = 0;
volatile uint32_t g_phase_inc = 0;

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
                    g_phase_inc = pgm_read_dword(&phase_lut[last_note]);
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