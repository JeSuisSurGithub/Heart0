#include "common.h"
#include "midi.h"
#include "dac.h"
#include "waves.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

typedef enum WAVEFORM {
    PW_SAW = 0,
    PW_SQU = 1,
    SAW = 2,
    SQU = 3,
    SINE = 4,
    CAMEL = 5,
    TRI = 6,
    TRI_SAW = 7,
    ABS = 8,
    HALF = 9,
    _MAX = 10
} WAVEFORM;


uint8_t wave_select = 0;
volatile uint16_t phase = 0;
volatile uint16_t cutoff = 32768;
volatile uint16_t env_decay = 1;
volatile uint16_t pwm = 0;
volatile bool pwm_dir = 0;
volatile uint16_t audio_sample;
volatile bool audio_ready = false;

uint16_t pwm_saw(uint16_t phase, uint16_t pw)
{
    uint16_t saw = phase >> 4;   // 0..4095

    uint16_t center = 2048;
    uint16_t half_pw = pw >> 1;

    if (saw > (center - half_pw) && saw < (center + half_pw)) {
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
    uint16_t value = pgm_read_byte(&wave_table[phase >> 8]) * 256;
    return value >> 4;
}

uint8_t button_pressed()
{
    static uint8_t last_state = 1;
    uint8_t current_state = (PIND & (1 << BUTTON_PIN)) != 0;

    if (last_state == 1 && current_state == 0)
    {
        _delay_ms(20);
        if ((PIND & (1 << BUTTON_PIN)) == 0)
        {
            last_state = 0;
            return 1;
        }
    }

    if (current_state == 1)
        last_state = 1;

    return 0;
}

uint16_t adc_read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
    return ADC;
}

int main(void) {
    init_midi();
    init_dac();

    // Normal mode
    // CTC, prescaler=1
    // 32KHz, compare match interrupt
    // Enable interrupt
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10);
    OCR1A = (F_CPU / (1 * SAMPLE_RATE)) - 1;
    TIMSK1 = (1 << OCIE1A);

    // Button pull-up
    DDRD  &= ~(1 << BUTTON_PIN);
    PORTD |=  (1 << BUTTON_PIN);

    // Env Decay
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    sei();
    while (true) {
        if (audio_ready) {
            audio_ready = false;
            dac_write(FILTER_CV, cutoff >> 4);
        }
        if (button_pressed()) {
            wave_select++;
            if (wave_select == _MAX) wave_select = 0;
        }
    uint16_t adc_env_decay = adc_read(0);
    env_decay = (adc_env_decay >> 6) + 1;
    }
}



ISR(TIMER1_COMPA_vect)
{
    // push last sample
    dac_write(WAVE, audio_sample);

    // phase update
    phase += g_phase_inc;

    // generate NEXT sample
    switch (wave_select) {
        case PW_SAW: audio_sample = pwm_saw(phase, pwm >> 4); break;
        case PW_SQU: audio_sample = pwm_squ(phase, pwm >> 4); break;
        case SAW: audio_sample = pwm_saw(phase, 0); break;
        case SQU: audio_sample = pwm_squ(phase, 2047); break;
        case SINE: audio_sample = read_wave(phase, sine_table); break;
        case CAMEL: audio_sample = read_wave(phase, camel_sine_table); break;
        case TRI: audio_sample = read_wave(phase, tri_table); break;
        case TRI_SAW: audio_sample = read_wave(phase, tri_saw_table); break;
        case ABS: audio_sample = read_wave(phase, abs_sine_table); break;
        case HALF: audio_sample = read_wave(phase, half_sine_table); break;
        default: audio_sample = 2048; break;
    }
    audio_ready = true;

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
        if (!pwm_dir) {
            pwm = pwm + 2;
            pwm_dir = pwm > 63488;
        } else {
            pwm = pwm - 2;
            pwm_dir = pwm > 2048;
        }
    }

    g_update_dac_flag = true;
}
