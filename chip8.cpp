#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include "chip8.h"
#include "config.h"

void Chip8::StoreInMemory(int offset, uint8_t to_save) {
    memory_[offset] = to_save;
}

void Chip8::StoreInMemory(int offset, uint16_t to_save) {
    memory_[offset] = to_save & 0xFF;
    memory_[offset + 1] = to_save >> 8;
}

bool Chip8::LoadProgram(const std::string filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    file.seekg(0, file.end);
    size_t size = file.tellg();
    file.seekg(0, file.beg);
    std::cout << "Read " << size << " bytes from file!" << std::endl;

    char c;
    for (uint16_t i = 0; file.get(c); i++) {
        StoreInMemory(0x200 + i, (uint8_t) c);
    }

    return true;
}

void Chip8::Execute() {    
    uint16_t opcode = memory_[program_counter_] << 8 | memory_[program_counter_ + 1];
    program_counter_ += 2;

    uint16_t first_nibble = opcode & 0xF000;

    switch (first_nibble) {
        // 0x00E0: Clear screen
        // 0x00EE: Subroutines
        // 0x0NNN: Ignored
        case 0x0000: {
            if (opcode == 0x00EE) {
                program_counter_ = stack_[--stack_pointer_];
            } else if (opcode == 0x00E0) {
                memset(display_data_.data(), 0, sizeof(display_data_));
            }
            break;
        }

        // 0x1NNN: Jump to address NNN
        case 0x1000: {
            program_counter_ = opcode & 0x0FFF;
            break;
        }

        // 0x2NNN: Subroutines
        case 0x2000: {
            stack_[stack_pointer_++] = program_counter_;
            program_counter_ = opcode & 0x0FFF;
            break;
        }

        // 0x3XNN
        case 0x3000: {
            uint8_t x = registers_[(opcode & 0x0F00) >> 8];
            uint8_t value = opcode & 0x00FF;

            if (x == value) {
                program_counter_ += 2;
            }
            break;
        }

        // 0x4XNN
        case 0x4000: {
            uint8_t x = registers_[(opcode & 0x0F00) >> 8];
            uint8_t value = opcode & 0x00FF;

            if (x != value) {
                program_counter_ += 2;
            }
            break;
        }

        // 0x5XY0
        case 0x5000: {
            uint8_t x = registers_[(opcode & 0x0F00) >> 8];
            uint8_t y = registers_[(opcode & 0x00F0) >> 4];

            if (x == y) {
                program_counter_ += 2;
            }
            break;
        }

        // 0x6XNN: Set register X to NN
        case 0x6000: {
            registers_[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        }

        // 0x7XNN: Add NN to register X
        case 0x7000: {
            registers_[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        }

        // 0x8XYR: Arithmetic Operations
        case 0x8000: {
            uint16_t last_nibble = opcode & 0x000F;
            uint8_t x = registers_[(opcode & 0x0F00) >> 8];
            uint8_t y = registers_[(opcode & 0x00F0) >> 4];

            switch (last_nibble) {
                case 0: 
                    registers_[(opcode & 0x0F00) >> 8] = y;
                    break;

                case 1: 
                    registers_[(opcode & 0x0F00) >> 8] |= y;
                    break;

                case 2:
                    registers_[(opcode & 0x0F00) >> 8] &= y;
                    break;

                case 3:
                    registers_[(opcode & 0x0F00) >> 8] ^= y;
                    break;

                case 4:
                    registers_[0xF] = x + y > 255 ? 1 : 0;
                    registers_[(opcode & 0x0F00) >> 8] += y;
                    break;

                case 5:
                    registers_[0xF] = x > y ? 1 : 0;
                    registers_[(opcode & 0x0F00) >> 8] = x - y;
                    break;

                case 6:
                    registers_[(opcode & 0x0F00) >> 8] = y >> 1;
                    registers_[0xF] = y & 1 ? 1 : 0;
                    break;

                case 7:
                    registers_[0xF] = x < y ? 1 : 0;
                    registers_[(opcode & 0x0F00) >> 8] = y - x;
                    break;

                case 0xE:
                    registers_[(opcode & 0x0F00) >> 8] = y << 1;
                    registers_[0xF] = y & (1 << 7) ? 1 : 0;
                    break;

            }

            break;
        }

        // 0x9XY0
        case 0x9000: {
            uint8_t x = registers_[(opcode & 0x0F00) >> 8];
            uint8_t y = registers_[(opcode & 0x00F0) >> 4];

            if (x != y) {
                program_counter_ += 2;
            }
            break;
        }

        // 0xA000: Set index register to NNN
        case 0xA000: {
            index_register_ = opcode & 0x0FFF;
            break;
        }

        // 0xDXYN: Draw sprite at (X, Y) with height N
        case 0xD000: {
            trigger_redraw_ = true;

            uint8_t x = registers_[(opcode & 0x0F00) >> 8] & 63;
            uint8_t y = registers_[(opcode & 0x00F0) >> 4] & 31;
            uint8_t height = opcode & 0x000F;
            registers_[0xF] = 0;

            uint8_t sprite;
            for (int i = 0; i < height && (i + y) < kDisplayHeight; i++) {
                sprite = memory_[index_register_ + i];
                for (int j = 0; j < 8 && (j + x) < kDisplayWidth; j++) {
                    uint8_t pixel = sprite & (0x80 >> j);
                    if (pixel > 0) {
                        if (display_data_[x + j][y + i] == 1) {
                            registers_[0xF] = 1;
                        }
                        display_data_[x + j][y + i] ^= 1;
                    }
                }
            }
            break;
        }
    }
}


Chip8::Chip8() {
    program_counter_ = kProgramStart;
    stack_pointer_ = 0;
    memset(display_data_.data(), 0, sizeof(display_data_));
    memset(memory_.data(), 0, sizeof(memory_));

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

