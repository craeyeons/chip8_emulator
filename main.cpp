#include <cstdint>
#include <string>
#include <iostream>

#include "raylib.h"
#include "chip8.h"
#include "config.h"


int main(int argc, char* argv[]) {
    Chip8 emulator;
    const std::string filename = argv[1];

    emulator.LoadProgram(filename);

    InitWindow(kDisplayWidth * kPixelSize, kDisplayHeight * kPixelSize, "Chip 8 Emulator");
    SetTargetFPS(kFPS);

    while (!WindowShouldClose())
    {
        emulator.Execute();
        if (emulator.trigger_redraw_) {
            emulator.trigger_redraw_ = false;

            BeginDrawing();
            ClearBackground(BLACK);

            for (int i = 0; i < kDisplayWidth; i++) {
                for (int j = 0; j < kDisplayHeight; j++) {
                    if (emulator.display_data_[i][j] == 1)
                        DrawRectangle(i * kPixelSize, j * kPixelSize, kPixelSize, kPixelSize, WHITE);
                }
            }

            EndDrawing();
        }
    }

    CloseWindow();

    return 0;
}