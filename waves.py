import numpy as np

n_samples = 256       # Table size
bits = 8              # 8-bit waveform
max_val = 2**bits - 1

t = np.arange(n_samples)

# -------------------------
sine = (np.sin(2*np.pi*t/n_samples)*0.5 + 0.5) * max_val
sine = sine.astype(np.uint8)

camel_sine = np.sin(2*np.pi*t/n_samples) + 0.5*np.sin(4*np.pi*t/n_samples)
camel_sine = np.clip((camel_sine*0.5 + 0.5)*max_val, 0, max_val)
camel_sine = camel_sine.astype(np.uint8)

tri = 2*np.abs(2*(t/n_samples) - 1) - 1
tri = ((tri*0.5 + 0.5)*max_val).astype(np.uint8)

abs_sine = np.abs(np.sin(2*np.pi*t/n_samples))
abs_sine = (abs_sine * max_val).astype(np.uint8)

half_sine = np.sin(2*np.pi*t/n_samples)
half_sine[half_sine < 0] = 0
half_sine = (half_sine * max_val).astype(np.uint8)

saw = t / n_samples            # 0..1
tri_saw = 0.5*saw + 0.5*(2*np.abs(2*(t/n_samples)-1)-1)/2 + 0.5  # normalize to 0..1
tri_saw = np.clip(tri_saw*max_val, 0, max_val).astype(np.uint8)

# -------------------------
# Function to print PROGMEM table
def print_table(name, data):
    print(f"const uint8_t {name}[{len(data)}] PROGMEM = {{")
    for i in range(0, len(data), 16):
        print(", ".join(f"{x}" for x in data[i:i+16]) + ("," if i+16 < len(data) else ""))
    print("};\n")

# Print all tables
print_table("sine_table", sine)
print_table("camel_sine_table", camel_sine)
print_table("tri_table", tri)
print_table("abs_sine_table", abs_sine)
print_table("half_sine_table", half_sine)
print_table("tri_saw_table", tri_saw)
