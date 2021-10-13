/* Test application for Yet Another Latency Measuring Device
 * Written in 2019 by Jürgen Hahn and Andreas Schmid (University of Regensburg)
 * Project website: https://hci.ur.de/projects/end-to-end-latency
 *
 * Simple program that reacts to a mouse click by changing the display's color from black to white.
 * Pass the number of measurements as an argument when starting the program.
 * Press 'Q' to exit the program.
 *
 * This is only a sample program written in SDL. When measuring the latency of your system,
 * you might want to use a different programming language and graphics framework.
 */

#include <SDL2/SDL.h>
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Usage: <binary> num_measurements");
        exit(1);
    }

    int num_iterations = atoi(argv[1]);

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(__FILE__, 0, 0, WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Event event;
    int measurement_iteration = 0;

    // loop until maximum number of measurements is reached
    while(measurement_iteration < num_iterations)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                // exit program when 'Q' is pressed
                if(event.key.keysym.sym == SDLK_q)
                {
                    measurement_iteration = num_iterations;
                    break;
                }
            }

            // on click, draw screen white
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                }
            }

            // on release, draw screen black
            if (event.type == SDL_MOUSEBUTTONUP)
            {
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    measurement_iteration++;
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);
    }

    SDL_Quit();

    return 0;
}
