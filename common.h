#ifndef COMMON_H
#define COMMON_H

#define F_CPU 8000000UL
#define SAMPLE_RATE 24000
#define PORT_RX PD0
#define PORT_VGATE PB1
#define PORT_CS PB2
#define PORT_SCLK PB5
#define PORT_MOSI PB3
#define BUTTON_PIN PD5


#include <stdbool.h>
#include <stdint.h>

// Input MIDI, Waveselect
// Output Gate, Waveform and CV
/**
    GND VCC AVCC AVREF
    PC6 RESET_N

    PB1 Gate (PWM)
    PB2-PB5 MCP4922 (CS MOSI MISO SCK)
    PD0 MIDI IN (UART RX)
    PD5 Switch Waveform

    PC0-PC3 Potentiometers
    PD6 PWM

    PB0 Unused Timer Capture
    PB6-PB7 Unused Crystal
    PC4-PC5 Unused I2C
    PD1 Unused UART TX
    PD2-PD3 Unused Int0, Int1
    PD4 Unused Timer0 Clock
    PD7 Unused Timer1 Clock
*/

#endif // COMMON_H