#include "headers/cpu.h"

char V[16]; // Registers
char dt{0}; // Delay timer
char st{0}; // Sound timer
char memory[4096]; // Emulated memory
short pc{0}; // Program counter

const short TIMER_HZ {60};


// Initialize cpu
void initializeCpu(){
    // Registers zeroed
    for (int i = 0; i < 16; i++)
    {
        V[i] = 0;
    }
    // Reset timers
    dt = 0;
    st = 0;
    // Memory zeroed
    for (int i = 0; i < 4096; i++)
    {
        memory[i] = 0;
    }

    // Initialize fontset
    
}

// Emulate cycle

// Update timers