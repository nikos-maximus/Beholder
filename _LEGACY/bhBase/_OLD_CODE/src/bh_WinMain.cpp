#if _WINDOWS // This is the #def to check wheter the _WINDOWS subsystem is in effect (as opposed to _CONSOLE) - It is not related to _WIN32/64

#include <Windows.h>
#include "Game/bhGame.hpp"

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	bhGame_Run();
	return 0;
}

#endif //_WINDOWS
