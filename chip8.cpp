#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include "chip8.h"
#include "config.h"

void Chip8::StoreInMemory(uint8_t offset, uint8_t to_save) {
    memory_[offset] = to_save;
}

void Chip8::StoreInMemory(uint8_t offset, uint16_t to_save) {
    memory_[offset] = to_save & 0xFF;
    memory_[offset + 1] = to_save >> 8;
}

bool Chip8::LoadProgram(const std::string filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    file.seekg(std::ios::end);
    size_t size = file.tellg();
    file.seekg(std::ios::beg);

    std::vector<uint8_t> program(size);
    file.read(reinterpret_cast<char*>(&program), size);

    for (uint16_t i = 0; i < size; i++) {
        StoreInMemory(0x200 + i, program[i]);
    }

    return true;
}


Chip8::Chip8() {
    program_counter_ = kProgramStart;

    for (int i = 0; i < 16 * 5; i++) {
        StoreInMemory(0x50 + 2 * i, kFonts[i]);
    }
}

void Chip8::TickTimers() {
    const std::chrono::steady_clock clock;
    std::chrono::time_point last_updated = clock.now();

    const std::chrono::milliseconds tick_duration = std::chrono::milliseconds(1000 / kTimerFrequency);

    while(true) {
        std::chrono::duration time_elpased = clock.now() - last_updated;
        last_updated = clock.now();

        if (time_elpased > tick_duration) {
            if (delay_timer_) {
                delay_timer_--;
            }

            if (sound_timer_) {
                sound_timer_--;
            }
        }
    }
}

