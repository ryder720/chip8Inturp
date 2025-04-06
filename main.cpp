#include "components/headers/cpu.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>


#define SDL_MAIN_HANDLED // Good practice, especially on Windows/macOS
#include <SDL2/SDL.h>

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

bool DEBUG = false;

// Window Dimensions
const int CHIP8_WIDTH = 64;
const int CHIP8_HEIGHT = 32;

// --- SDL Display Constants ---
const int SCALE_FACTOR = 15; // Increase this for a larger window
const int SCREEN_WIDTH = CHIP8_WIDTH * SCALE_FACTOR;
const int SCREEN_HEIGHT = CHIP8_HEIGHT * SCALE_FACTOR;

void loadRom(std::string filePath = "./roms/logo.ch8"){
    
    std::filesystem::path romPath{filePath};
    if(std::filesystem::exists(romPath)){
        int fileSize = std::filesystem::file_size(romPath);
        std::fstream file(filePath);
        printf("File size: %d\n",fileSize);

        // Make a buffer of the exact size of the file and read the data into it.
        std::vector<u_char> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()),fileSize);
        for (int i = 0; i < fileSize; i++)
            {
                
                memory[i + 0x200] = buffer[i]; 
                if (DEBUG == true){
                    std::cout << "Added to memory address " << i << "\n";
                }
            }
            

        std::cout << "ROM Loaded\n";
        file.close();
    } else
    {
        std::cout << "No ROM Could not be found!!!!\n";
    }
    
    
    
}


int main(int argc, char* argv[]){

    // Initialize SDL 
    // We only need video subsystem for windows and events
    std::cout << "Initializing Window\n";
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "Creating Window\n";
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Chip-8 Emulator",      // Title that appears on the window border
        SDL_WINDOWPOS_UNDEFINED,  // Let the OS decide the initial X position
        SDL_WINDOWPOS_UNDEFINED,  // Let the OS decide the initial Y position
        SCREEN_WIDTH,             // Width of the window's client area in pixels
        SCREEN_HEIGHT,            // Height of the window's client area in pixels
        SDL_WINDOW_SHOWN          // Flags: Make sure the window is visible upon creation
    );
    // Check if window creation failed
    if (window == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit(); // Clean up initialized SDL subsystems
        return 1;   // Return error code
    }
    std::cout << "Window Created\n";
    std::cout << "Creating Renderer\n";
    // Create Renderer
    // A renderer is needed to draw graphics onto the window.
    // -1: Use the first rendering driver supporting the flags
    // SDL_RENDERER_ACCELERATED: Use hardware acceleration if available
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
         std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
         SDL_DestroyWindow(window); // Clean up the window
         SDL_Quit(); // Clean up SDL
         return 1;
    }
    std::cout << "Renderer Created\n";
    std::cout << "Initializing Screen\n";
    // Initialize Screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    std::cout << "Screen Initialized\n";

    std::cout << "Initializing CPU\n";
    // Initialize CPU
    initializeCpu();
    std::cout << "Loading ROM\n";
    // Load rom into memory
    if (argc > 1){
        loadRom(argv[1]);
    } else{
        loadRom();
    }

    SDL_Event event;
    bool playing = true;
    std::cout << "Program Start\n";
    while (playing)
    {
        // Run system
        emulateCycle();

        // update timers on 60hz clock

        
        
        // Update graphics
        while(SDL_PollEvent(&event) != 0){
            if (event.type == SDL_QUIT){
                playing = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                // Handle key events
                if (event.key.keysym.sym == SDLK_ESCAPE){
                    playing = false;
                }
            }
            
        }
        if(drawFlag == true){
            // Draw every pixel on the screen
            // 1. Clear screen (set background color)
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); // Black
            SDL_RenderClear(renderer);

            // 2. Set drawing color for pixels that are ON
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // White

            // 3. Iterate through the CHIP-8 graphics buffer
            for (int y = 0; y < CHIP8_HEIGHT; ++y) {
                for (int x = 0; x < CHIP8_WIDTH; ++x) {
                    // Calculate the index in the 1D gfx array
                    int index = y * CHIP8_WIDTH + x;

                    // If the pixel at (x, y) is ON (value is 1)
                    if (gfx[index] == 1) {
                        // Create a rectangle for the scaled pixel
                        SDL_Rect pixelRect;
                        pixelRect.x = x * SCALE_FACTOR;
                        pixelRect.y = y * SCALE_FACTOR;
                        pixelRect.w = SCALE_FACTOR;
                        pixelRect.h = SCALE_FACTOR;

                        // Draw the filled rectangle
                        SDL_RenderFillRect(renderer, &pixelRect);
                    }
                    // No need to draw if the pixel is OFF, because we already cleared to black
                }
            }
            SDL_RenderPresent(renderer);
            drawFlag = false;
        }
        
        
        
    }
    std::cout << "Program End\n";

    return 0;
}