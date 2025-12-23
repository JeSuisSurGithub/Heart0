#ifndef MIDI_H
#define MIDI_H

#include "common.h"


extern volatile uint8_t g_note_on_flag;
extern volatile uint16_t g_current_phase_inc; // actual phase increment
extern volatile uint16_t g_target_phase_inc;  // set by MIDI note-on


void init_midi();

#endif // MIDI_H