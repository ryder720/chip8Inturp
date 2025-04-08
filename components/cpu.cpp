#include "headers/cpu.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

unsigned char V[16]; // Registers
unsigned char dt{0}; // Delay timer
unsigned char st{0}; // Sound timer
unsigned char memory[4096]; // Emulated memory
unsigned short pc{0}; // Program counter
unsigned short I{0}; // Index register
unsigned short opcode{0};

unsigned short stack[16]; //Stack
unsigned short sp; // Stack pointer

unsigned char key[16]; // Keypad key pressed

unsigned char gfx[32 * 64];
bool drawFlag{false};

unsigned char fontset[80] ={
// Initialize fontset
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

const short TIMER_HZ {60};




// Initialize cpu
void initializeCpu(){
    // Reset graphics
    for (int i = 0; i < 2048; i++)
    {
        gfx[i] = 0;
    }
    
    // Registers zeroed
    for (int i = 0; i < 16; i++)
    {
        V[i] = 0x00;
    }
    // Reset timers
    dt = 0;
    st = 0;
    // Memory zeroed
    for (int i = 0; i < 4096; i++)
    {
        memory[i] = 0;
    }
    // Program counter set to begining of rom memory
    pc = 0x200;

    // Load fontset into memory
    for (int i = 0; i < 80; i++)
    {
        memory[i + 0x050] = fontset[i];
    }
    

    // Set rng seed
    std::srand(std::time(0));

    std::cout << "CPU Initialized\n";
}

int generateRandomIntFromZero(int max){
    return std::rand() % (max + 1);
}

void clearKeys(){
    for (int i = 0; i < 16; i++)
    {
        key[i] = 0;
    }
    
}

void updateKey(int keyPressed){
    clearKeys();
    key[keyPressed] = 1;
}




// Emulate cycle
void emulateCycle(){
    // Get instruction in memory using program counter
    // Grabbing 2 bytes as that's the size of each opcode
    // We then murge using bitwise OR aka |
    // Using << 8 to shift the first byte over 8 bits
    opcode = memory[pc] << 8 | memory[pc + 1];
    bool keyNotFound = true;

    // Get the first 4 bits by bitwise AND with 0xF000
    switch (opcode & 0xF000u)
    {
        case 0x0000:
            // Multiple 0x0 opcodes
            switch (opcode & 0x000F) 
            {
            case 0x0000u: // 0x00E0 Clear screen
                printf("0x00E0 Clear Screen OPCODE: %0X\n", opcode);

                // Clear screen

                
                pc += 2; // Move program counter
                break;
            
            case 0x000Eu: // 0x000E Return from subrutine
                printf("0x00EE Return from Subroutine OPCODE: %0X\n", opcode);
                // Get last pointer from stack
                pc = stack[sp - 1];
                sp--;
                break;
            
            default:
                printf("Unknown OPCODE: %0X\n", opcode);
                break;
            }
            break;
        case 0x1000u: // 1NNN Jump to location 1NNN
            printf("1NNN Jump to location 1NNN OPCODE: %0X\n", opcode);
            pc = opcode & 0x0FFFu;
            break;
        case 0x2000u: // 2NNN Call subroutine at 2NNN
            printf("2NNN Call Subroutine at 2NNN OPCODE: %0X\n", opcode);
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFFu;
            break;
        case 0x3000u: // 3XKK Skip Next instruction if Vx == kk
            printf("3XKK Skip next instruction if Vx == kk OPCODE: %0X\n", opcode);
            if(V[(opcode & 0x0F00u) >> 8] == (opcode & 0x00FFu)){
                pc += 4;
            }
            else{
                pc += 2;
            }
            break;
        case 0x4000u: // 3XKK Skip Next instruction if Vx != kk
            printf("3XKK Skip next instruction if Vx != kk OPCODE: %0X\n", opcode);
            if(V[(opcode & 0x0F00u) >> 8] == (opcode & 0x00FFu)){
                pc += 2;
            }
            else{
                pc += 4;
            }
            break;
        case 0x6000u: // 6XKK Set Vx = kk
            printf("6XKK Set Vx = kk OPCODE: %0X\n", opcode);
            V[(opcode & 0x0F00u) >> 8] = opcode & 0x00FFu;
            pc += 2;
            break;
        case 0x7000u: // 7XKK Set Vx = Vx + kk
            printf("7XKK Set Vx = Vx + kk OPCODE: %0X\n", opcode);
            V[(opcode & 0x0F00u) >> 8] += opcode & 0x00FFu;
            pc += 2;
            break;
        case 0x8000u: // Multiple math op codes
                switch (opcode & 0x000Fu){
                    case 0x0000u: // 8XY0 Set the value of Vy into Vx
                        printf("8XY0 Set Vx = kk OPCODE: %0X\n", opcode);
                        V[(opcode & 0x0F00u) >> 8] = V[(opcode & 0x00F0u) >> 4];
                        pc += 2;
                        break;
                    case 0x0002u: // 8XY2 Set Vx = Vx AND Vy
                        printf("8XY2 Set Vx = Vx AND Vy OPCODE: %0X\n", opcode);
                        V[(opcode & 0x0F00u) >> 8] = V[(opcode & 0x0F00u) >> 8] & V[(opcode & 0x00F0u) >> 4];
                        pc += 2;
                        break;
                    case 0x0004u: // 8XY4 Set Vx = Vx + Vy, set VF = carry
                        printf("8XY4 Set Vx = Vx + Vy, set VF = carry OPCODE: %0X\n", opcode);
                        if ((uint16_t)(V[(opcode & 0x0F00u) >> 8]) + (uint16_t)(V[(opcode & 0x00F0u) >> 4]) > 0xFF)
                        {
                            V[0xFu] = 1;
                        } else {
                            V[0xFu] = 0;
                        }

                        V[(opcode & 0x0F00u) >> 8] += V[(opcode & 0x00F0u) >> 4];
                        pc += 2;
                        break;
                    default:
                        printf("Unknown OPCODE: %0X\n", opcode);
                        break;
                }
            break;
        
        case 0xA000u: // ANNN: set I to address NNN
            printf("ANNN Set I to NNN OPCODE: %0X\n", opcode);
            I = opcode & 0x0FFFu;
            // Move to next instruction
            pc += 2;
            break;
        
        case 0xC000u: // CXKK: Set V[x] to random byte & kk
            printf("CXKK: Set V[x] to random byte & kk OPCODE: %0X\n", opcode);
            V[(opcode & 0x0F00u) >> 8] = generateRandomIntFromZero(255) & (opcode & 0x00FFu);

            pc += 2;
            break;
        
        case 0xD000u:{ // DXYN: Displaying sprites
            printf("DXYN Display n-byte sprite at memory location I OPCODE: %0X\n", opcode);
            // A doozy This instruction starts by reading n bytes starting at I
            // These are displayed as sprites at the location (Vx,Vy)
            // Sprites are XOR'd onto the existing screen which can cause VF to set to 1
            // If a pixel would be placed outside of the screen it would then be wrapped around
            // Extract coordinates and height from opcode
        unsigned short x_coord_reg = (opcode & 0x0F00u) >> 8; // Index for Vx
        unsigned short y_coord_reg = (opcode & 0x00F0u) >> 4; // Index for Vy
        unsigned short start_x = V[x_coord_reg]; // Get starting X from Vx
        unsigned short start_y = V[y_coord_reg]; // Get starting Y from Vy
        unsigned short height = opcode & 0x000Fu;  // Height of the sprite (N)

        unsigned char pixel_row_data; // To store one byte (row) of sprite data

        // Reset collision flag VF
        V[0xF] = 0;

        // Loop through each row of the sprite (height pixels high)
        for (int yline = 0; yline < height; ++yline) {
            // Prevent reading past memory bounds (simple check)
            if (I + yline >= 4096) continue;

            // Read the byte representing the current sprite row
            pixel_row_data = memory[I + yline];

            // Loop through the 8 bits (pixels) in this row
            for (int xline = 0; xline < 8; ++xline) {
                // Check if the current bit in the sprite data is set (1)
                // (0x80 >> xline) creates masks: 10000000, 01000000, ...
                if ((pixel_row_data & (0x80u >> xline)) != 0) {
                    // Calculate target screen coordinates with wrapping
                    int targetX = (start_x + xline) % 64;
                    int targetY = (start_y + yline) % 32;

                    // Calculate the 1D index for the gfx buffer
                    int gfxIndex = targetX + (targetY * 64);

                     // Prevent potential out-of-bounds access if modulo wasn't perfect (shouldn't happen with %)
                    if (gfxIndex >= sizeof(gfx)) continue;

                    // Check for collision: If the screen pixel is already 1
                    if (gfx[gfxIndex] == 1) {
                        V[0xF] = 1; // Set the collision flag
                    }

                    // Draw the pixel using XOR
                    gfx[gfxIndex] ^= 1;
                }
            }
        }
            drawFlag = true;
            pc += 2;
            break;
        }

        case 0xE000u: // Multiple 0xE opcodes
            switch (opcode & 0x00FFu){
                case 0x00A1u: // EXA1: Skip next instruction if key with the value of Vx is not pressed
                printf("EXA1 Skip next instruction if key with the value of Vx is not pressed OPCODE: %0X\n", opcode);
                    // Check if key[i] is down or up
                    if (key[(opcode & 0x0F00u) >> 8] == 1)
                    {
                        pc += 2;
                    } else {
                        pc += 4;
                    }
                    break;
                default:
                    break;
            }

            break;
        
        case 0xF000u: // Multiple 0xF opcodes
            switch (opcode & 0x00FFu)
            {
            case 0x0007u: // FX07: Set Vx to the value of the delay timer
                printf("FX07 Set Vx = DT OPCODE: %0X\n", opcode);
                V[(opcode & 0x0F00u) >> 8] = dt;
                pc += 2;
                break;
            case 0x000Au: // FX0A: Wait for and store key press in Vx
                printf("FX0A Wait for and store key press in Vx OPCODE: %0X\n", opcode);
                int keyFound;
                for (int i = 0; i < 16; i++)
                {
                    if (key[i] == 1)
                    {
                        keyNotFound = false;
                        keyFound = i;
                        
                    }
                    
                }
                
                if(keyNotFound == true){
                    break;
                }
                V[(opcode & 0x0F00u) >> 8] = key[keyFound];
                pc += 2;
                break;
            case 0x0015u: // FX15: Set the delay timer to the value of Vx
                printf("FX15 Set DT = Vx OPCODE: %0X\n", opcode);
                dt = V[(opcode & 0x0F00u) >> 8];
                pc += 2;
                break;
            case 0x0018u: // FX18: Set the sound timer to the value of Vx
                printf("FX15 Set ST = Vx OPCODE: %0X\n", opcode);
                st = V[(opcode & 0x0F00u) >> 8];
                pc += 2;
                break;
            case 0x0029u: // FX29: Set I = location of sprite for digit Vx
                printf("FX29 Set I = location of sprite for digit Vx OPCODE: %0X\n", opcode);
                // V[x] is the hexcode for the digit I want to display
                I = 0x050u + V[((opcode & 0x0F00u) >> 8) * 5];

                pc += 2;
                break;
            case 0x0033u: // FX33: Store BCD representation of Vx in memory locations I, I+1, and I+2
                printf("FX33 Store BCD representation of Vx in memory locations I, I+1, and I+2: %0X\n", opcode);
                memory[I] = V[(opcode & 0x0F00u) >> 8] / 100;
                memory[I + 1] = (V[(opcode & 0x0F00u) >> 8]  / 10) % 10;
                memory[I + 2] = (V[(opcode & 0x0F00u) >> 8] % 100) % 10;
                pc += 2;
                break;
            case 0x0065u: // FX65: Read registers V0 through Vx from memory starting at location I
                printf("FX65 Read registers V0 through Vx from memory starting at location I OPCODE: %0X\n", opcode);
                for (unsigned int i = 0; i < ((opcode & 0x0F00u) >> 8); i++)
                {
                    V[i] = I + i;
                }
                pc += 2;
                break;
                
            default:
                printf("Unknown OPCODE: %0X\n", opcode);
                break;
            }
        break;

        default:
            printf("Unknown OPCODE: %0X\n", opcode);
            break;
    }
}

