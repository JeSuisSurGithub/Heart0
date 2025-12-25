#include "dac.h"
#include "midi.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define CONTROL_FREQ 1000

volatile uint16_t cutoff = 32768;
volatile uint16_t env_decay = 1;
volatile bool pwm_direction = 0;

uint16_t adc_read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

int main()
{
    init_midi();
    init_dac();

    // Timer0
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS02) | (1 << CS00); // /1024
    OCR0A  = (F_CPU / 1024 / CONTROL_FREQ) - 1; // ~1 kHz
    TIMSK0 = (1 << OCIE0A);

    // Button pull-up
    DDRD  &= ~(1 << BUTTON_PIN);
    PORTD |=  (1 << BUTTON_PIN);

    // Env Decay
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    sei();
    while (true) {
        static uint8_t btn_state = 0;

        uint8_t pressed = !(PIND & (1 << BUTTON_PIN));

        if (pressed) {
            if (!btn_state) {
                btn_state = 1;
                g_wave_select++;
                if (g_wave_select == _MAX)
                    g_wave_select = 0;
            }
        } else {
            btn_state = 0;
        }
        uint16_t adc_env_decay = adc_read(0);
        env_decay = (adc_env_decay >> 1) + 1;
        _delay_ms(10);
    }
}


ISR(TIMER0_COMPA_vect)
{
    if (g_phase_inc < g_phase_inc_target) {
        uint16_t delta = g_phase_inc_target - g_phase_inc;
        if (delta > g_glide_rate)
            g_phase_inc += g_glide_rate;
        else
            g_phase_inc = g_phase_inc_target;
    }
    else if (g_phase_inc > g_phase_inc_target) {
        uint16_t delta = g_phase_inc - g_phase_inc_target;
        if (delta > g_glide_rate)
            g_phase_inc -= g_glide_rate;
        else
            g_phase_inc = g_phase_inc_target;
    }

    if (g_update_keypress) {
        g_update_keypress = false;
        cutoff = 32768;
    }

    if (cutoff > env_decay) {
        cutoff = cutoff - env_decay;
    } else {
        cutoff = 0;
    }

    if (g_note_on_flag) {
        if (!pwm_direction) {
            g_pwm = g_pwm + 64;
            pwm_direction = g_pwm > 63488;
        } else {
            g_pwm = g_pwm - 64;
            pwm_direction = g_pwm > 2048;
        }
    }
    dac_write(FILTER_CV, cutoff >> 4);
}

