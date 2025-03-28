#include <SDL.h>
#include "Software/bhRaycaster.h"
#include "Software/bhPrimitives.h"

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        int window_Width = 800, window_Height = 600;

        SDL_Window* mainWindow = NULL;
        mainWindow = SDL_CreateWindow("RayCaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_Width, window_Height, 0);
        if (mainWindow != NULL)
        {
            {
                bhRaycaster_Init(window_Width, window_Height);

                int running = 1;
                SDL_Event evt;
                while (running)
                {
                    while (SDL_PollEvent(&evt))
                    {
                        if (evt.type == SDL_QUIT)
                        {
                            running = 0;
                        }
                    }

                    bhRaycaster_HandleInput();

                    SDL_Surface* wndSurface = SDL_GetWindowSurface(mainWindow);
                    SDL_assert(wndSurface != NULL);
                    if (SDL_MUSTLOCK(wndSurface))
                    {
                        SDL_LockSurface(wndSurface);
                    }
                    bhPaintTestGradient(wndSurface, 0, 0);
                    bhRaycaster_DrawMap(wndSurface);
                    if (SDL_MUSTLOCK(wndSurface))
                    {
                        SDL_UnlockSurface(wndSurface);
                    }
                    SDL_UpdateWindowSurface(mainWindow);
                }

                bhRaycaster_Destroy();
            }
            SDL_DestroyWindow(mainWindow);
        }
        SDL_Quit();
    }
    return 0;
}
