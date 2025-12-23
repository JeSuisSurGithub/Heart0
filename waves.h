#ifndef WAVES_H
#define WAVES_H
#include "common.h"
#include <avr/pgmspace.h>

extern const uint8_t sine_table[256] PROGMEM;
extern const uint8_t camel_sine_table[256] PROGMEM;
extern const uint8_t tri_table[256] PROGMEM;
extern const uint8_t abs_sine_table[256] PROGMEM;
extern const uint8_t half_sine_table[256] PROGMEM;
extern const uint8_t tri_saw_table[256] PROGMEM;

#endif //WAVES_H