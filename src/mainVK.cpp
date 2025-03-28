#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include "bhConfig.hpp"

SDL_Window* g_mainWnd = nullptr;
VkInstance g_instance = VK_NULL_HANDLE;

bool InitVulkan()
{
  if (!SDL_Vulkan_LoadLibrary(nullptr)) return false;

  VkApplicationInfo appInfo =
  {
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "Beholder",
    0,
    "BeholderEngine",
    0,
    0
  };
  
  VkInstanceCreateInfo instanceCI =
  {
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    nullptr,
    0,
    &appInfo,
    0,
    nullptr,
    0,
    nullptr
  };
  
  vkCreateInstance(&instanceCI, nullptr, &g_instance);
  return g_instance != VK_NULL_HANDLE;
}

void DestroyVulkan()
{
  //vkDeviceWaitIdle()
  vkDestroyInstance(g_instance, nullptr);
  SDL_Vulkan_UnloadLibrary();
}

int main(int argc, char* argv[])
{
  bool running = true;

  if (SDL_Init(SDL_INIT_VIDEO))
  {
    bhConfig cfg;
    //bhConfig::Load()
    SDL_PropertiesID props = bhConfig::CreateProperties(cfg);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);

    if (InitVulkan())
    {
      g_mainWnd = SDL_CreateWindowWithProperties(props);
      if (g_mainWnd)
      {
        SDL_ShowWindow(g_mainWnd); // TODO: Error check
        while (running)
        {
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
            }
          }
        }
        SDL_DestroyWindow(g_mainWnd);
      }
      DestroyVulkan();
    }

    SDL_Quit();
    return 0;
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
}
