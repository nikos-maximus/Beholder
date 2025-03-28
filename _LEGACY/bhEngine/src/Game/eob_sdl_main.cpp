#include "SDL/SurfaceSDL.hpp"

int main(int argc, char* argv[])
{
  int sdlErr = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  if (sdlErr < 0)
  {
    return sdlErr;
  }

  Uint32 wndFlags = SDL_WINDOW_HIDDEN;
  SDL_Window* mainWnd = nullptr;
  SDL_Renderer* renderer = nullptr;
  int result = SDL_CreateWindowAndRenderer("EoB SDL", 800, 450, wndFlags, &mainWnd, &renderer);
  if (result == 0) //Success
  {
    SDL_SetWindowPosition(mainWnd, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED); // TODO: Check return

	SDL_SetRenderVSync(renderer, 1); //Sync every frame
    SurfaceSDL wndSurf;
    wndSurf.Create(mainWnd);

      SurfaceSDL imgSurf;
      imgSurf.Create("D:/Pictures/female-superhero-asterix-obelix.jpg");

    imgSurf.BlitToSurface(wndSurf);

    bool running = true;
    SDL_ShowWindow(mainWnd);

    while (running)
    {
      SDL_PumpEvents();
      SDL_Event evt;
      while (SDL_PollEvent(&evt))
      {
        switch (evt.type)
        {
          case SDL_EVENT_QUIT:
          {
            running = false;
            break;
          }
          default:
          {
            break;
          }
        }
      }
      SDL_UpdateWindowSurface(mainWnd);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(mainWnd);
  }

  SDL_Quit();
  return 0;
}
