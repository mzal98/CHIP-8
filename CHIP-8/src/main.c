#include <stdio.h>

#include "chip8.h"


int main() {

    chip8Initialize();
    
    for (;;)
    {
        chip8EmulateCycle();

    }

    return 0;
}

