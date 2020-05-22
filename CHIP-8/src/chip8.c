#include "chip8.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#pragma warning(disable : 4996)

void chip8Initialize()
{
    chip8 = (chip8_t){ 0 };
    chip8.pc = 0x200;

    chip8.drawFlag = true;
    addCharset();
    chip8.sp = 0x00;

    chip8.compatibilityMode = false;

    srand(time(0));
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

    memcpy(&chip8.memory[0x0], chip8_fontset, 80);
}

void chip8EmulateCycle()
{
    chip8.opcode = chip8.memory[chip8.pc] << 8 | chip8.memory[chip8.pc + 1];
    executeOpcode();

    if (chip8.delay_timer > 0)
        --chip8.delay_timer;

    if (chip8.sound_timer > 0)
    {
        if (chip8.sound_timer == 1)
            printf("BEEP!\n");
        --chip8.sound_timer;
    }

}

static void executeOpcode()
{
#define OPVAR_X ((chip8.opcode & 0x0F00) >> 8)
#define OPVAR_Y ((chip8.opcode & 0x00F0) >> 4)
#define OPVAR_N (chip8.opcode & 0x000F)
#define OPVAR_KK (chip8.opcode & 0x00FF)
#define OPVAR_NNN (chip8.opcode & 0x0FFF)

bool keyPressed = false;

    switch(chip8.opcode & 0xF000)
    {
        case 0x0000:
            switch (chip8.opcode)
            {
                // Clear display, gfx
                case 0x00E0:
                    memset(chip8.gfx, 0x0, 2048);
                    chip8.drawFlag = true;
                    chip8.pc += 2;
                    break;

                // Return from subroutine
                case 0x00EE:
                    chip8.pc = chip8.stack[--chip8.sp];
                    chip8.pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", chip8.opcode);
                    break;
            }
            break;

        // Jump to NNN
        case 0x1000:
            chip8.pc = OPVAR_NNN;
            break;

        // Call subroutine at NNN
        case 0x2000:
            chip8.stack[chip8.sp++] = chip8.pc;
            chip8.pc = OPVAR_NNN;
            break;

        // Skip next instruction if V[x] == kk
        case 0x3000:
            if (chip8.V[OPVAR_X] == OPVAR_KK)
                chip8.pc += 4;
            else
                chip8.pc += 2;
            break;

        // Skip next instruction if V[x] != kk
        case 0x4000:
            if (chip8.V[OPVAR_X] != OPVAR_KK)
                chip8.pc += 4;
            else
                chip8.pc += 2;
            break;

        // Skip next instruction if V[x] == V[y]
        case 0x5000:
            if (chip8.V[OPVAR_X] == chip8.V[OPVAR_Y])
                chip8.pc += 4;
            else
                chip8.pc += 2;
            break;

        // Load V[x] with kk
        case 0x6000:
            chip8.V[OPVAR_X] = OPVAR_KK;
            chip8.pc += 2;
            break;

        // Add kk to V[x]
        case 0x7000:
            chip8.V[OPVAR_X] += OPVAR_KK;
            chip8.pc += 2;
            break;

        case 0x8000:
            switch (OPVAR_N)
            {
                // V[x] = V[y]
                case 0x0000:
                    chip8.V[OPVAR_X] = chip8.V[OPVAR_Y];
                    chip8.pc += 2;
                    break;

                // V[x] = V[x] or V[y]
                case 0x0001:
                    chip8.V[OPVAR_X] |= chip8.V[OPVAR_Y];
                    chip8.pc += 2;
                    break;

                // V[x] = V[x] and V[y]
                case 0x0002:
                    chip8.V[OPVAR_X] &= chip8.V[OPVAR_Y];
                    chip8.pc += 2;
                    break;
                
                // V[x] = V[x] xor V[y]
                case 0x0003:
                    chip8.V[OPVAR_X] ^= chip8.V[OPVAR_Y];
                    chip8.pc += 2;
                    break;

                // V[x] = V[x] and V[y]
                case 0x0004:
                    chip8.V[0xF] = (chip8.V[OPVAR_Y] > (0xFF - chip8.V[OPVAR_X]));
                    chip8.V[OPVAR_X] += chip8.V[OPVAR_Y];
                    chip8.pc += 2;
                    break;

                // V[x] = V[x] - V[y]
                case 0x0005:
                    chip8.V[0xF] = !(chip8.V[OPVAR_Y] > chip8.V[OPVAR_X]);
                    chip8.V[OPVAR_X] -= chip8.V[OPVAR_Y];
                    chip8.pc += 2;
                    break;

                // V[x] shift 1 to the right, store LSB in V[F]
                // Compatibility Mode: V[x] = V[y] >> 1, V[F] = LSB V[y]
                case 0x0006:
                    if (chip8.compatibilityMode)
                    {
                        printf("8XY6 Compatibility Mode\n");
                        chip8.V[0xF] = chip8.V[OPVAR_Y] & 0x1;
                        chip8.V[OPVAR_X] = chip8.V[OPVAR_Y] >> 1;
                    }
                    else {
                        chip8.V[0xF] = chip8.V[OPVAR_X] & 0x1;
                        chip8.V[OPVAR_X] >>= 1;
                    }
                    chip8.pc += 2;
                    break;

                // V[x] = V[y] - V[x], store borrow in V[F]
                case 0x0007:
                    chip8.V[0xF] = !(chip8.V[OPVAR_X] > chip8.V[OPVAR_Y]);
                    chip8.V[OPVAR_X] = chip8.V[OPVAR_Y] - chip8.V[OPVAR_X];
                    chip8.pc += 2;
                    break;

                // V[x] shift left by one, store MSB in V[F]
                // Compatibility Mode: V[x] = V[y] << 1, V[F] = MSB V[y]
                case 0x000E:
                    if (chip8.compatibilityMode)
                    {
                        printf("8XYE Compatibility Mode\n");
                        chip8.V[0xF] = chip8.V[OPVAR_Y] >> 7;
                        chip8.V[OPVAR_X] = chip8.V[OPVAR_Y] << 1;
                    }
                    else
                    {
                        chip8.V[0xF] = chip8.V[OPVAR_X] >> 7;
                        chip8.V[OPVAR_X] <<= 1;
                    }
                    chip8.pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", chip8.opcode);
                    break;
            }
            break;

        // Skip next instruction if V[x] != V[y]
        case 0x9000:
            if (chip8.V[OPVAR_X] != chip8.V[OPVAR_Y])
                chip8.pc += 4;
            else
                chip8.pc += 2;
            break;

        // I = nnn
        case 0xA000:
            chip8.I = OPVAR_NNN;
            chip8.pc += 2;
            break;

        // Jump to nnn + V[0]
        case 0xB000:
            chip8.pc = OPVAR_NNN + chip8.V[0];
            break;

        // V[x] = random byte and kk
        case 0xC000:
            chip8.V[OPVAR_X] = (rand() % 0xFF) & OPVAR_KK;
            chip8.pc += 2;
            break;

        // Display n-byte sprite at I at (V[x], V[y]), set V[F] = 1 if collision
        case 0xD000:

            chip8.V[0xF] = 0;

            for (int row = 0; row < OPVAR_N; row++)
            {
                uint8_t pixel = chip8.memory[chip8.I + row];

                for (int col = 0; col < 8; col++)
                {
                    if ((pixel & (0x80 >> col)) != 0)
                    {
                        int index = (chip8.V[OPVAR_X] + col + ((chip8.V[OPVAR_Y] + row) * 64)) % (64 * 32);

                        if (chip8.gfx[index] == 1)
                            chip8.V[0xF] = 1;

                        chip8.gfx[index] ^= 1;
                    }
                }
            }
            chip8.drawFlag = true;
            chip8.pc += 2;
            break;

        case 0xE000:
            switch (chip8.opcode & 0x00FF)
            {
                // Skip next instructon if key[x] is pressed
                case 0x009E:
                    if (chip8.key[OPVAR_X] != 0)
                        chip8.pc += 4;
                    else
                        chip8.pc += 2;
                    break;

                // Skip next instructon if key[x] is not pressed
                case 0x00A1:
                    if (chip8.key[OPVAR_X] == 0)
                        chip8.pc += 4;
                    else
                        chip8.pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", chip8.opcode);
                    break;
            }
            break;

        case 0xF000:
            switch (chip8.opcode & 0x00FF)
            {
                // Vx = delay timer
                case 0x0007:
                    chip8.V[OPVAR_X] = chip8.delay_timer;
                    chip8.pc += 2;
                    break;

                // Stop execution until key pressed, store key press in V[x]
                case 0x000A:
                    for (int i = 0; i < 16; i++)
                    {
                        if (chip8.key[i] != 0)
                        {
                            chip8.V[OPVAR_X] =  i;
                            keyPressed = true;
                        }
                    }

                    if (!keyPressed)
                        return;

                    chip8.pc += 2;
                    break;

                // Set delay timer = V[x]
                case 0x0015:
                    chip8.delay_timer = chip8.V[OPVAR_X];
                    chip8.pc += 2;
                    break;

                // Set sound timer = V[x]
                case 0x0018:
                    chip8.sound_timer = chip8.V[OPVAR_X];
                    chip8.pc += 2;
                    break;

                // I = I + V[x]
                case 0x001E:
                    chip8.V[0xF] = !(chip8.I + chip8.V[OPVAR_X] > 0xFFF);
                    chip8.I += chip8.V[OPVAR_X];
                    chip8.pc += 2;
                    break;

                // Set I = sprite location from V[x]
                case 0x0029:
                    chip8.I = chip8.V[OPVAR_X] * 0x5;
                    chip8.pc += 2;
                    break;

                // Store BCD of V[x] into address I, I + 1 and I + 2
                case 0x0033:
                    chip8.memory[chip8.I] = chip8.V[OPVAR_X] / 100;
                    chip8.memory[chip8.I + 1] = (chip8.V[OPVAR_X] / 10) % 10;
                    chip8.memory[chip8.I + 2] = (chip8.V[OPVAR_X] % 100) % 10;
                    chip8.pc += 2;
                    break;

                // Store all V into address I+
                // Compatibility Mode: I incremented by # of registers stored
                case 0x0055:
                    for (size_t i = 0; i <= OPVAR_X; i++)
                        chip8.memory[chip8.I + i] = chip8.V[i];
                    if (chip8.compatibilityMode)
                    {
                        printf("8X55 Compatibility Mode\n");
                        chip8.I += (OPVAR_X + 1);
                    }
                    chip8.pc += 2;
                    break;

                // Store all saved registers from I to V
                // Compatibility Mode: I incremented by # of registers read
                case 0x0065:
                    for (size_t i = 0; i <= OPVAR_X; i++)
                        chip8.V[i] = chip8.memory[chip8.I + i];
                    if (chip8.compatibilityMode)
                    {
                        printf("8X65 Compatibility Mode\n");
                        chip8.I += (OPVAR_X + 1);
                    }
                    chip8.pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", chip8.opcode);
                    break;
            }
            break;

        default:
            printf("Unknown opcode: 0x%X\n", chip8.opcode);
            break;
    }
}

bool chip8LoadRom(char fileName[])
{
    unsigned int fileLength = 0;
    FILE* rom = fopen(fileName, "rb");

    if (rom == NULL)
    {
        fprintf(stderr, "Opening %s: %s\n", fileName, strerror(errno));
        return false;
    }

    fseek(rom, 0, SEEK_END);
    fileLength = ftell(rom);
    fseek(rom, 0, SEEK_SET);
    fread(chip8.memory + 0x200, fileLength, 1, rom);
    fclose(rom);
    printf("%s Loaded Successfully\n", fileName);
    return true;
}

void chip8EnableCompatibilityMode()
{
    chip8.compatibilityMode = true;
}