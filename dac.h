#ifndef DAC_H
#define DAC_H

#include "common.h"

#define MID_VALUE 2048

typedef enum DAC_CH {
    WAVE= 0,
    FILTER_CV = 1,
}DAC_CH;

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
}WAVEFORM;

extern volatile uint16_t audio_sample;
extern volatile uint16_t phase;

extern volatile uint8_t g_wave_select;
extern volatile uint16_t g_pwm;

void dac_write(DAC_CH channel, uint16_t value);
void init_dac();

#endif // DAC_H