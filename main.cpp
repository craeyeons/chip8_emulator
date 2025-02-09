#include <cstdint>
#include <string>

#include "raylib.h"
#include "chip8.h"
#include "config.h"

int main(int argc, char* argv[]) {
    Chip8 emulator;
    const std::string filename = argv[1];

    InitWindow(kDisplayWidth, kDisplayHeight, "Chip 8 Emulator");

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}