#ifndef DAC_H
#define DAC_H

#include "common.h"

extern volatile bool g_update_dac_flag;

typedef enum DAC_CH {
    WAVE= 0,
    FILTER_CV = 1,
}DAC_CH;

void dac_write(DAC_CH channel, uint16_t value);
void init_dac();

#endif // DAC_H