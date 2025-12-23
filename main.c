#include "common.h"
#include "dac.h"
#include "midi.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

typedef enum WAVEFORM{
    SAW = 0,
    SQU = 1,
    PW_SAW = 2,
    PW_SQU = 3,
    _MAX = 4
}WAVEFORM;

uint8_t wave_select = 0;
volatile uint32_t phase = 0;
volatile uint16_t cutoff = 0;
volatile uint16_t pwm = 0;
volatile bool pwm_dir = 0;

uint16_t pwm_saw(uint32_t phase, uint16_t pw)
{
    uint16_t saw = phase >> 20;   // 0..4095

    uint16_t center = 2048;
    uint16_t half_pw = pw >> 1;

    if (saw > (center - half_pw) && saw < (center + half_pw)) {
        return 0;   // absolute minimum
    }
    else {
        return saw;
    }
}

uint16_t pwm_squ(uint32_t phase, uint16_t pw)
{
    uint16_t squ = phase >> 20;   // 0..4095

    if (squ > pw) {
        return 0;
    }
    else {
        return 4095;
    }
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

    sei();
    while (true) {
        if (g_update_dac_flag) {
            g_update_dac_flag = false;
            switch (wave_select) {
                case SAW:
                    dac_write(WAVE, pwm_saw(phase, 0));
                    break;
                case SQU:
                    dac_write(WAVE, pwm_squ(phase, 2047));
                    break;
                case PW_SAW:
                    dac_write(WAVE, pwm_saw(phase, pwm >> 4));
                    break;
                case PW_SQU:
                    dac_write(WAVE, pwm_squ(phase, pwm >> 4));
                    break;
                default:
                    break;
            }
            dac_write(FILTER_CV, cutoff >> 4);
        }
        if (button_pressed()) {
            wave_select++;
            if (wave_select == _MAX) wave_select = 0;
        }
    }
}



ISR(TIMER1_COMPA_vect) {
    phase += g_phase_inc;
    if (g_note_on_flag)  {
        cutoff = (cutoff + 1) % 2048;
        if (!pwm_dir) {
            pwm = pwm + 1;
            pwm_dir = pwm > 49152;
        } else {
            pwm = pwm - 1;
            pwm_dir = pwm > 16384;
        }
    }
    g_update_dac_flag = true;
}
