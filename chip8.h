#ifndef CHIP8_H_
#define CHIP8_H_

#include <cstdint>
#include <array>
#include <string>

#include "config.h"

class Chip8 {
public:
    Chip8();
    bool LoadProgram(const std::string filename);
    void Execute();
private:
    std::array<uint8_t, kMemorySize> memory_;
    uint16_t program_counter_;
    uint16_t index_register_;
    uint16_t stack_pointer_;
    std::array<uint16_t, kStackSize> stack_;
    std::array<uint8_t, kNumberOfRegisters> registers_;
    
    uint8_t delay_timer_;
    uint8_t sound_timer_;

    void TickTimers();
    void StoreInMemory(uint8_t offset, uint8_t to_save);
    void StoreInMemory(uint8_t offset, uint16_t to_save);
};

#endif
