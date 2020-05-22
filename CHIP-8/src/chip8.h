#pragma once
#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
typedef struct chip8
{
    /* CPU */
    uint16_t opcode;
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    bool compatibilityMode;

    /* Memory */
    uint8_t memory[4096];

    /* Stack */
    uint16_t stack[16];
    uint16_t sp;

    /* Display */
    uint8_t gfx[64 * 32];
    bool drawFlag;

    /* Timers */
    uint8_t delay_timer;
    uint8_t sound_timer;

    /* Keypad */
    uint8_t key[16];
} chip8_t;
chip8_t chip8;
static void addCharset();
void chip8Initialize();
void chip8EmulateCycle();
static void executeOpcode();
bool chip8LoadRom(char[]);
void chip8EnableCompatibilityMode();


#endif // !CHIP8_H