#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include "raylib.h"
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

void Chip8::LogKeyPresses() {
    for (int i = 0; i < kNumberOfKeys; i++) {
        if (IsKeyDown(kKeys[i])) {
            key_presses_[i] = 1;
        } else if (IsKeyUp(kKeys[i])) {
            key_presses_[i] = 0;
        }
    }
}

void Chip8::Execute() {   
    TickTimers();
    LogKeyPresses();

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
                    registers_[(opcode & 0x0F00) >> 8] += y;
                    registers_[0xF] = x + y > 255 ? 1 : 0;
                    break;

                case 5:
                    registers_[(opcode & 0x0F00) >> 8] = x - y;
                    registers_[0xF] = x >= y ? 1 : 0;
                    break;

                case 6:
                    registers_[(opcode & 0x0F00) >> 8] = y >> 1;
                    registers_[0xF] = y & 1 ? 1 : 0;
                    break;

                case 7:
                    registers_[(opcode & 0x0F00) >> 8] = y - x;
                    registers_[0xF] = x <= y ? 1 : 0;
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

        // 0xB000: Jump with offset
        case 0xB000: {
            uint16_t offset = opcode & 0x0FFF;
            program_counter_ = registers_[0x0] + offset;
            break;
        }

        // 0xCXNN: Random
        case 0xC000: {
            // To Change, generate random int
            uint8_t random_number = 0x1F;
            uint8_t offset = opcode & 0xFF;
            registers_[(opcode & 0x0F00) >> 8] = random_number | offset;
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

        // 0xEX9E and EXA1: Skip if key
        case 0xE000: {
            uint8_t register_index = (opcode & 0xF00) >> 8;
            uint8_t corresponding_key = registers_[register_index];
            uint8_t is_pressed = key_presses_[corresponding_key];
            uint8_t last_two_nibbles = opcode & 0xFF;

            switch (last_two_nibbles) {
                case 0x9E: {
                    if (is_pressed) {
                        program_counter_ += 2;
                    }
                    break;
                }
                
                case 0xA1: {
                    if (!is_pressed) {
                        program_counter_ += 2;
                    }
                    break;
                }
            }
            break;
        }

        case 0xF000: {
            uint8_t register_index = (opcode & 0xF00) >> 8;
            uint8_t last_two_nibbles = opcode & 0xFF;
            switch (last_two_nibbles) {
                case 0x07: {
                    registers_[register_index] = delay_timer_;
                    break;
                }

                case 0x15: {
                    delay_timer_ = registers_[register_index];
                    break;
                }

                case 0x18: {
                    sound_timer_ = registers_[register_index];
                    break;
                }

                case 0x1E: {
                    index_register_ += registers_[register_index];
                    break;
                }

                case 0x0A: {
                    bool flag = false;
                    for (int i = 0; i < kNumberOfKeys; i++) {
                        if (key_presses_[i]) {
                            registers_[register_index] = i;
                            flag = true;
                            break;
                        }
                    }
                    program_counter_ = flag ? program_counter_ : program_counter_ - 2;
                    break;
                }

                case 0x29: {
                    index_register_ = 0x50 + registers_[register_index] * 5;
                    break;
                }

                case 0x33: {
                    uint8_t number = registers_[register_index];
                    memory_[index_register_] = (number / 100) % 10;
                    memory_[index_register_ + 1] = (number / 10) % 10;
                    memory_[index_register_ + 2] = number % 10;
                    break;
                }

                case 0x55: {
                    uint8_t limit = register_index;
                    for (int i = 0; i <= limit; i++) {
                        memory_[index_register_ + i] = registers_[i];
                    }
                    break;
                }

                case 0x65: {
                    uint8_t limit = register_index;
                    for (int i = 0; i <= limit; i++) {
                        registers_[i] = memory_[index_register_ + i];
                    }
                    break;
                }
            }
            break;
        }
    }
}


Chip8::Chip8() {
    program_counter_ = kProgramStart;
    stack_pointer_ = 0;
    last_updated_ = std::chrono::steady_clock::now();
    memset(display_data_.data(), 0, sizeof(display_data_));
    memset(memory_.data(), 0, sizeof(memory_));
    memset(key_presses_.data(), 0, sizeof(key_presses_));
    memset(registers_.data(), 0, sizeof(registers_));

    for (int i = 0; i < 16 * 5; i++) {
        StoreInMemory(0x50 + 2 * i, kFonts[i]);
    }
}

void Chip8::TickTimers() {
    std::chrono::time_point<std::chrono::steady_clock> current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_time = current_time - last_updated_;
    while (elapsed_time > kTickInterval) {
        if (delay_timer_ > 0) {
            delay_timer_--;
        }

        if (sound_timer_ > 0) {
            sound_timer_--;
        }

        last_updated_ += kTickInterval;
        elapsed_time = current_time - last_updated_;
    }
}

