#pragma once
#ifndef CHIP8_H
#define CHIP8_H

static void addCharset();
void chip8Initialize();
void chip8EmulateCycle();
static void executeOpcode();
void chip8TestRom();


#endif // !CHIP8_H