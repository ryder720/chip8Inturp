#include "components/headers/cpu.h"


/*
Chip-8 Emulator
Specs:
16 8 bit registers V0-VF
16 bit index register
64 byte stack
64 x 32 display
4096 bits of memory
8 bit stack pointer
8 bit delay timer
8 bit sound timer
64 x 32 array frame buffer

fontset takes up 0x0 - 0x080
programs start at 0x200
*/


int main(){
    // Initialize CPU
    initializeCpu();
    // Load rom into memory

    return 0;
}