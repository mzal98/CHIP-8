#include "chip8.h"
#include "graphics.h"

int main() {   

    chip8Initialize();
    //chip8EnableCompatibilityMode(); // Uncomment to enable
    chip8LoadRom("Pong.ch8");
    initGraphics();
    for (;;)
    {
        chip8EmulateCycle();
        updateGraphics();
    }

    return 0;
}

