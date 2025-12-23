import math

# Configuration
SAMPLE_RATE = 32000     # default sample rate in Hz
A4_FREQ = 432           # tuning frequency for A4
TABLE_SIZE = 128        # number of MIDI notes
PHASE_BITS = 16         # number of bits for phase accumulator

def midi_to_freq(midi_note, a4_freq=A4_FREQ):
    """Convert MIDI note to frequency."""
    return a4_freq * 2 ** ((midi_note - 69) / 12)

def phase_increment(freq, sample_rate=SAMPLE_RATE, phase_bits=PHASE_BITS):
    """
    Compute the 32-bit phase increment for a given frequency.
    Formula: increment = freq * 2^phase_bits / sample_rate
    """
    return int((freq * (1 << phase_bits)) / sample_rate)

def generate_phase_lut():
    """Generate phase increment lookup table for all 128 MIDI notes."""
    lut = [phase_increment(midi_to_freq(note)) for note in range(TABLE_SIZE)]
    return lut

def format_lut_c(lut):
    """Format the LUT as a C-style PROGMEM array."""
    lines = []
    for i in range(0, len(lut), 8):
        line = ", ".join(f"{v}" for v in lut[i:i+8])
        lines.append("    " + line)
    return "const uint16_t phase_lut[128] PROGMEM = {\n" + ",\n".join(lines) + "\n};"

if __name__ == "__main__":
    lut = generate_phase_lut()
    c_code = format_lut_c(lut)
    print(c_code)
