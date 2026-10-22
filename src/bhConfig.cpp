#include <stdio.h>
#include <SDL3/SDL_video.h>
#include "bhConfig.hpp"

size_t bhConfig::Load(const char* path, bhConfig& cfg)
{
  size_t result{ 0 };
  FILE* file = fopen(path, "rb");
  if (file)
  {
    constexpr size_t confSiz = sizeof(bhConfig);
    if (fread(&cfg, confSiz, 1, file) == confSiz) { result = confSiz; }
    fclose(file);
  }
  return result;
}

size_t bhConfig::Save(const char* path, const bhConfig& cfg)
{
  size_t result{ 0 };
  FILE* file = fopen(path, "wb");
  if (file)
  {
    constexpr size_t confSiz = sizeof(bhConfig);
    if (fwrite(&cfg, confSiz, 1, file) == confSiz) { result = confSiz; }
    fclose(file);
  }
  return result;
}

SDL_PropertiesID bhConfig::CreateProperties(const bhConfig& config)
{
  SDL_PropertiesID props = SDL_CreateProperties();
  if (props)
  {
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, config.window.fullscreen);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);

    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, config.window.w);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, config.window.h);

    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);

    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
  }
  return props;
}

void bhConfig::DestroyProperties(SDL_PropertiesID props)
{
  SDL_DestroyProperties(props);
}
