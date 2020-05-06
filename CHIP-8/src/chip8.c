#include "chip8.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct memory
{
    /* Memory */
    uint8_t mem[4096];
    bool romLoaded;
} Memory;

static Memory memory;

static void initializeMemory()
{
    memory = (Memory){ 0 };
    addCharset();
}

static void addCharset()
{
    uint8_t chip8_fontset[80] =
    {
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

    memcpy(&memory.mem[0x0], chip8_fontset, 80);
}

typedef struct stack
{
    /* Stack */
    uint16_t stack[16];
    uint16_t sp;
} Stack;

static Stack stack;

static void initializeStack()
{
    stack = (Stack){ 0 };
    stack.sp = 0xAA;
}

typedef struct cpu
{
    /* CPU */
    uint16_t opcode;
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;

    /* Display */
    uint8_t gtx[64 * 32];

    /* Timers */
    uint8_t delay_timer;
    uint8_t sound_timer;

    /* Keypad */
    uint8_t key[16];

    bool drawFlag;

} CPU;

static CPU cpu;

static void initializeCPU() 
{
    cpu = (CPU){ 0 };
    cpu.pc = 0x200;

    cpu.drawFlag = true;
}

void chip8Initialize()
{
    initializeCPU();
    initializeMemory();
    initializeStack();

    srand(time(0));
}