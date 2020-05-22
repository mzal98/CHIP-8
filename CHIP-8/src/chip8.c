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
                    chip8.V[0xF] = chip8.V[OPVAR_Y] > (0xFF - chip8.V[OPVAR_X]);
                    chip8.V[OPVAR_X] = (chip8.V[OPVAR_X] + chip8.V[OPVAR_Y]) & 0x00FF;
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
                    if (chip8.I + chip8.V[OPVAR_X] > 0xFFF)
                        chip8.V[0xF] = 1;
                    else
                        chip8.V[0xF] = 0;
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
                    break;
            }
            break;

        default:
            printf("Unknown opcode: 0x%X\n", chip8.opcode);
            break;

// Process opcode
//switch (chip8.opcode & 0xF000)
//{
//case 0x0000:
//    switch (OPVAR_N)
//    {
//    case 0x0000: // 0x00E0: Clears the screen
//        for (int i = 0; i < 2048; ++i)
//            chip8.gfx[i] = 0x0;
//        chip8.drawFlag = true;
//        chip8.pc += 2;
//        break;
//
//    case 0x000E: // 0x00EE: Returns from subroutine
//        --chip8.sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
//        chip8.pc = chip8.stack[chip8.sp];	// Put the stored return address from the stack back into the program counter					
//        chip8.pc += 2;		// Don't forget to increase the program counter!
//        break;
//
//    default:
//        printf("Unknown opcode [0x0000]: 0x%X\n", chip8.opcode);
//    }
//    break;
//
//case 0x1000: // 0x1NNN: Jumps to address NNN
//    chip8.pc = OPVAR_NNN;
//    break;
//
//case 0x2000: // 0x2NNN: Calls subroutine at NNN.
//    chip8.stack[chip8.sp] = chip8.pc;			// Store current address in stack
//    ++chip8.sp;					// Increment stack pointer
//    chip8.pc = OPVAR_NNN;	// Set the program counter to the address at NNN
//    break;
//
//case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
//    if (chip8.V[(chip8.opcode & 0x0F00) >> 8] == (chip8.opcode & 0x00FF))
//        chip8.pc += 4;
//    else
//        chip8.pc += 2;
//    break;
//
//case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
//    if (chip8.V[(chip8.opcode & 0x0F00) >> 8] != (chip8.opcode & 0x00FF))
//        chip8.pc += 4;
//    else
//        chip8.pc += 2;
//    break;
//
//case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
//    if (chip8.V[(chip8.opcode & 0x0F00) >> 8] == chip8.V[(chip8.opcode & 0x00F0) >> 4])
//        chip8.pc += 4;
//    else
//        chip8.pc += 2;
//    break;
//
//case 0x6000: // 0x6XNN: Sets VX to NN.
//    chip8.V[(chip8.opcode & 0x0F00) >> 8] = chip8.opcode & 0x00FF;
//    chip8.pc += 2;
//    break;
//
//case 0x7000: // 0x7XNN: Adds NN to VX.
//    chip8.V[(chip8.opcode & 0x0F00) >> 8] += chip8.opcode & 0x00FF;
//    chip8.pc += 2;
//    break;
//
//case 0x8000:
//    switch (chip8.opcode & 0x000F)
//    {
//    case 0x0000: // 0x8XY0: Sets VX to the value of VY
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] = chip8.V[(chip8.opcode & 0x00F0) >> 4];
//        chip8.pc += 2;
//        break;
//
//    case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] |= chip8.V[(chip8.opcode & 0x00F0) >> 4];
//        chip8.pc += 2;
//        break;
//
//    case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] &= chip8.V[(chip8.opcode & 0x00F0) >> 4];
//        chip8.pc += 2;
//        break;
//
//    case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] ^= chip8.V[(chip8.opcode & 0x00F0) >> 4];
//        chip8.pc += 2;
//        break;
//
//    case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't					
//        if (chip8.V[(chip8.opcode & 0x00F0) >> 4] > (0xFF - chip8.V[(chip8.opcode & 0x0F00) >> 8]))
//            chip8.V[0xF] = 1; //carry
//        else
//            chip8.V[0xF] = 0;
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] += chip8.V[(chip8.opcode & 0x00F0) >> 4];
//        chip8.pc += 2;
//        break;
//
//    case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
//        if (chip8.V[(chip8.opcode & 0x00F0) >> 4] > chip8.V[(chip8.opcode & 0x0F00) >> 8])
//            chip8.V[0xF] = 0; // there is a borrow
//        else
//            chip8.V[0xF] = 1;
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] -= chip8.V[(chip8.opcode & 0x00F0) >> 4];
//        chip8.pc += 2;
//        break;
//
//    case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
//        chip8.V[0xF] = chip8.V[(chip8.opcode & 0x0F00) >> 8] & 0x1;
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] >>= 1;
//        chip8.pc += 2;
//        break;
//
//    case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
//        if (chip8.V[(chip8.opcode & 0x0F00) >> 8] > chip8.V[(chip8.opcode & 0x00F0) >> 4])	// VY-VX
//            chip8.V[0xF] = 0; // there is a borrow
//        else
//            chip8.V[0xF] = 1;
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] = chip8.V[(chip8.opcode & 0x00F0) >> 4] - chip8.V[(chip8.opcode & 0x0F00) >> 8];
//        chip8.pc += 2;
//        break;
//
//    case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
//        chip8.V[0xF] = chip8.V[(chip8.opcode & 0x0F00) >> 8] >> 7;
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] <<= 1;
//        chip8.pc += 2;
//        break;
//
//    default:
//        printf("Unknown opcode [0x8000]: 0x%X\n", chip8.opcode);
//    }
//    break;
//
//case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
//    if (chip8.V[(chip8.opcode & 0x0F00) >> 8] != chip8.V[(chip8.opcode & 0x00F0) >> 4])
//        chip8.pc += 4;
//    else
//        chip8.pc += 2;
//    break;
//
//case 0xA000: // ANNN: Sets I to the address NNN
//    chip8.I = chip8.opcode & 0x0FFF;
//    chip8.pc += 2;
//    break;
//
//case 0xB000: // BNNN: Jumps to the address NNN plus V0
//    chip8.pc = (chip8.opcode & 0x0FFF) + chip8.V[0];
//    break;
//
//case 0xC000: // CXNN: Sets VX to a random number and NN
//    chip8.V[(chip8.opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (chip8.opcode & 0x00FF);
//    chip8.pc += 2;
//    break;
//
//case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
//             // Each row of 8 pixels is read as bit-coded starting from memory location I; 
//             // I value doesn't change after the execution of this instruction. 
//             // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, 
//             // and to 0 if that doesn't happen
//{
//    unsigned short x = chip8.V[(chip8.opcode & 0x0F00) >> 8];
//    unsigned short y = chip8.V[(chip8.opcode & 0x00F0) >> 4];
//    unsigned short height = chip8.opcode & 0x000F;
//    unsigned short pixel;
//
//    chip8.V[0xF] = 0;
//    for (int yline = 0; yline < height; yline++)
//    {
//        pixel = chip8.memory[chip8.I + yline];
//        for (int xline = 0; xline < 8; xline++)
//        {
//            if ((pixel & (0x80 >> xline)) != 0)
//            {
//                if (chip8.gfx[(x + xline + ((y + yline) * 64))] == 1)
//                {
//                    chip8.V[0xF] = 1;
//                }
//                chip8.gfx[x + xline + ((y + yline) * 64)] ^= 1;
//            }
//        }
//    }
//
//    chip8.drawFlag = true;
//    chip8.pc += 2;
//}
//break;
//
//case 0xE000:
//    switch (chip8.opcode & 0x00FF)
//    {
//    case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
//        if (chip8.key[chip8.V[(chip8.opcode & 0x0F00) >> 8]] != 0)
//            chip8.pc += 4;
//        else
//            chip8.pc += 2;
//        break;
//
//    case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
//        if (chip8.key[chip8.V[(chip8.opcode & 0x0F00) >> 8]] == 0)
//            chip8.pc += 4;
//        else
//            chip8.pc += 2;
//        break;
//
//    default:
//        printf("Unknown opcode [0xE000]: 0x%X\n", chip8.opcode);
//    }
//    break;
//
//case 0xF000:
//    switch (chip8.opcode & 0x00FF)
//    {
//    case 0x0007: // FX07: Sets VX to the value of the delay timer
//        chip8.V[(chip8.opcode & 0x0F00) >> 8] = chip8.delay_timer;
//        chip8.pc += 2;
//        break;
//
//    case 0x000A: // FX0A: A key press is awaited, and then stored in VX		
//    {
//        bool keyPress = false;
//
//        for (int i = 0; i < 16; ++i)
//        {
//            if (chip8.key[i] != 0)
//            {
//                chip8.V[(chip8.opcode & 0x0F00) >> 8] = i;
//                keyPress = true;
//            }
//        }
//
//        // If we didn't received a keypress, skip this cycle and try again.
//        if (!keyPress)
//            return;
//
//        chip8.pc += 2;
//    }
//    break;
//
//    case 0x0015: // FX15: Sets the delay timer to VX
//        chip8.delay_timer = chip8.V[(chip8.opcode & 0x0F00) >> 8];
//        chip8.pc += 2;
//        break;
//
//    case 0x0018: // FX18: Sets the sound timer to VX
//        chip8.sound_timer = chip8.V[(chip8.opcode & 0x0F00) >> 8];
//        chip8.pc += 2;
//        break;
//
//    case 0x001E: // FX1E: Adds VX to I
//        if (chip8.I + chip8.V[(chip8.opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
//            chip8.V[0xF] = 1;
//        else
//            chip8.V[0xF] = 0;
//        chip8.I += chip8.V[(chip8.opcode & 0x0F00) >> 8];
//        chip8.pc += 2;
//        break;
//
//    case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
//        chip8.I = chip8.V[(chip8.opcode & 0x0F00) >> 8] * 0x5;
//        chip8.pc += 2;
//        break;
//
//    case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
//        chip8.memory[chip8.I] = chip8.V[(chip8.opcode & 0x0F00) >> 8] / 100;
//        chip8.memory[chip8.I + 1] = (chip8.V[(chip8.opcode & 0x0F00) >> 8] / 10) % 10;
//        chip8.memory[chip8.I + 2] = (chip8.V[(chip8.opcode & 0x0F00) >> 8] % 100) % 10;
//        chip8.pc += 2;
//        break;
//
//    case 0x0055: // FX55: Stores V0 to VX in memory starting at address I					
//        for (int i = 0; i <= ((chip8.opcode & 0x0F00) >> 8); ++i)
//            chip8.memory[chip8.I + i] = chip8.V[i];
//
//        // On the original interpreter, when the operation is done, I = I + X + 1.
//        chip8.I += ((chip8.opcode & 0x0F00) >> 8) + 1;
//        chip8.pc += 2;
//        break;
//
//    case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I					
//        for (int i = 0; i <= ((chip8.opcode & 0x0F00) >> 8); ++i)
//            chip8.V[i] = chip8.memory[chip8.I + i];
//
//        // On the original interpreter, when the operation is done, I = I + X + 1.
//        chip8.I += ((chip8.opcode & 0x0F00) >> 8) + 1;
//        chip8.pc += 2;
//        break;
//
//    default:
//        printf("Unknown opcode [0xF000]: 0x%X\n", chip8.opcode);
//    }
//    break;
//
//default:
//    printf("Unknown opcode: 0x%X\n", chip8.opcode);
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