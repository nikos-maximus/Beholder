#include <SDL.h>
#include "xp/bhRendererSW.h"
//#include "bhLog.h"
//#include "bhEnv.h"
#include "tests_World.h"
#include "Game/bhDungeon.h"

#if 0
#define NUM_SPAWN_JOBS 512
int numbers[NUM_SPAWN_JOBS];

bool ThreadJob(void* data)
{
    printf("Thread id: %d - Job: %d\n", std::this_thread::get_id(), *reinterpret_cast<uint32_t*>(data));
    return true;
}

void TestThreads()
{
    for (uint32_t j = 0; j < NUM_SPAWN_JOBS; ++j)
    {
        numbers[j] = j;
        bhThreading::SpawnJob(ThreadJob, &(numbers[j]));
    }
}

int main(int argc, char* argv[])
{
    if (bhThreading::Init())
    {
        TestThreads();
        bhThreading::Destroy();
    }
    return 0;
}
#endif

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        //bhEnv::SetApplicationName("Software tests");
        if (bhRendererSW::Init())
        {
            tests_World::Init(600, 400, bhRendererSW::GetWindowFormat());
            bool running = true;
            SDL_Event evt;
            while (running)
            {
                while (SDL_PollEvent(&evt))
                {
                    switch (evt.type)
                    {
                        case SDL_QUIT:
                        {
                            running = false;
                            break;
                        }
                        //case SDL_WINDOWEVENT:
                        //{
                        //    switch (evt.window.event)
                        //    {
                        //        case SDL_WINDOWEVENT_CLOSE:
                        //        {
                        //            running = false;
                        //            break;
                        //        }
                        //        default:
                        //        {
                        //            break;
                        //        }
                        //    }
                        //    break;
                        //}
                        default:
                        {
                            break;
                        }
                    }
                }
                bhRendererSW::BeginFrame();
                //tests_World::Tick();
                //bhRendererSW::DEBUG_TestBlit(tests_World::GetRenderSurface());
                tests_World::Render();

                //SDL_Rect sr = { 0, 0, 100, 100 };
                //SDL_Rect dr = { 50, 50, 100, 100 };
                //bhRendererSW::BlitSurface(tests_World::GetRenderSurface(), &sr, &dr);
                bhRendererSW::BlitSurface(tests_World::GetRenderSurface(), nullptr, nullptr);
                bhRendererSW::EndFrame();
            }
            bhRendererSW::Destroy();
        }
        SDL_Quit();
    }
    return 0;
}

#if 0
int main(int argc, char* argv[])
{
    bhDungeon dungeon(32, 32);
    dungeon.Generate();
    //dungeon.DEBUG_Print();
    return 0;
}
#endif
