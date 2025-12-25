#ifndef MIDI_H
#define MIDI_H

#include "common.h"

#define MIDI_BAUD 31250
#define NOTE_ON 0x90
#define NOTE_OFF 0x80

extern volatile bool g_note_on_flag;
extern volatile bool g_update_keypress;
extern volatile uint16_t g_phase_inc;
extern volatile uint16_t g_phase_inc_target;
extern volatile uint16_t glide_rate;


void init_midi();

#endif // MIDI_H