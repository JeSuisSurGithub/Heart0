#ifndef MIDI_H
#define MIDI_H

#include "common.h"


extern volatile uint8_t g_note_on_flag;
extern volatile uint32_t g_phase_inc;

void init_midi();

#endif // MIDI_H