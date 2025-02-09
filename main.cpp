#include <cstdint>
#include <string>
#include <SDL.h>

#include "chip8.h"
#include "config.h"

int main(int argc, char* argv[]) {
    Chip8 emulator;
    const std::string filename = argv[1];

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Chip 8 Emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, kDisplayWidth, kDisplayHeight, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    while(true) {

    }
}