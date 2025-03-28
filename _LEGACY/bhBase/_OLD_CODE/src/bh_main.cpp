#include "Game/bhGame.hpp"

#if !_WINDOWS // This is the #def to check wheter the _WINDOWS subsystem is in effect (as opposed to _CONSOLE) - It is not related to _WIN32/64

int main(int argc, const char* argv[])
{
    bhGame_Run();
    return 0;
}

#endif
