#include "graphics.h"
#include "chip8.h"
#include <stdio.h>
#include <SDL.h>

SDL_Window* window;
SDL_Renderer* renderer;

int initGraphics() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return -1;
    }

	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64 * 9, 32 * 9, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        fprintf(stderr, "Graphics window failed to initialise: %s\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        fprintf(stderr, "Graphics renderer failed to initialise: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    return 0;
}

void updateGraphics() 
{
	if (chip8.drawFlag) 
	{
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);
		int row = -9;
		int col = 0;

		for (int i = 0; i < 64 * 32; i++) {
			if ((i % 64) == 0) {
				row += 9;
				col = 0;
			}
			if (chip8.gfx[i] == 1) {
				SDL_Rect pixelRect = { col, row, 10, 10 };
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderFillRect(renderer, &pixelRect);
			}
			col += 9;
		}
		SDL_RenderPresent(renderer);
		chip8.drawFlag = false;
	}


}