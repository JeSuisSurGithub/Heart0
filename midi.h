#ifndef MIDI_H
#define MIDI_H

#include "common.h"


extern volatile uint8_t g_note_on_flag;
extern volatile uint16_t g_phase_inc;
extern volatile bool g_update_keypress;

void init_midi();

#endif // MIDI_H