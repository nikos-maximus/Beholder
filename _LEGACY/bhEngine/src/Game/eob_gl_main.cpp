#include <GL/gl3w.h>
#include <SDL3/SDL.h>

static constexpr int OPENGL_VERSION_MAJOR = 4;
static constexpr int OPENGL_VERSION_MINOR = 5;

void SetupOpenGLContext()
{
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_VERSION_MAJOR);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_VERSION_MINOR);

#ifdef _DEBUG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}

int main(int argc, char* argv[])
{
  int sdlErr = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  if (sdlErr < 0)
  {
    return sdlErr;
  }

  SetupOpenGLContext();

  Uint32 wndFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
  SDL_Window* mainWnd = SDL_CreateWindow("EoB OpenGL", 800, 450, wndFlags);
  if (mainWnd)
  {
    SDL_SetWindowPosition(mainWnd, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED); // TODO: Check return

    SDL_GLContext glContext = SDL_GL_CreateContext(mainWnd);
    int gl3wResult = gl3wInit();
    if (gl3wResult)
    {
      return gl3wResult;
    }
    gl3wResult = gl3wIsSupported(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
    if (!gl3wResult)
    {
      return -1;
    }

    glClearColor(1.0f, 0.0, 1.0f, 1.0f);

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
      glClear(GL_COLOR_BUFFER_BIT);
      SDL_GL_SwapWindow(mainWnd);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(mainWnd);
  }

  SDL_Quit();
  return 0;
}
