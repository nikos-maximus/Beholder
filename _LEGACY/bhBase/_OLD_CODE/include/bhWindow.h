#ifndef BH_WINDOW_H
#define BH_WINDOW_H

#include <SDL.h>

#ifdef SDL_h_
typedef SDL_Window bhWindow;
#endif

#ifdef __cplusplus
extern "C" {
#endif

bhWindow* bhCreateWindow(const char* windowTitle); // Returns 0 on success, < 0 on fail
void bhDestroyWindow(bhWindow* wnd);
void bhShowWindow(bhWindow* wnd);
void bhHideWindow(bhWindow* wnd);
void bhGetWindowSize(bhWindow* wnd, int* w, int* h);
float bhGetWindowAspect(bhWindow* wnd);

#ifdef __cplusplus
}
#endif //extern "C"

#endif //BH_WINDOW_H
