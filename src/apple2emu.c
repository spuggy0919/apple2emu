#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ui.h"
/*
 * apple ii main,SDL event loop and screen render refresh
 * author:spuggy0919
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 650
#define CELL_SIZE 50
#define GRID_SIZE 8

SDL_Window* window;
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL initialization error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("APPLE II Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED ); //SDL_RENDERER_ACCELERATED
    if (!renderer) {
        printf("Renderer creation error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    srand(time(NULL)); // Initialize random seed

    UIMAIN_Initial(renderer);

    SDL_Event event;
    int quit = 0;
        // Clear the screen
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_RenderClear(renderer);
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            UIMAIN_Event(event);
        }
        if (!trigger_30Hz){

            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);
     

            // Draw grid cells with their colors
            int update=0;
            UIMAIN_Refresh(&update);
            // Update the screen
            SDL_RenderPresent(renderer); 
        }
    }

    UIMAIN_Release();
    // Cleanup and shutdown SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}