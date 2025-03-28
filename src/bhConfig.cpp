#include <stdio.h>
#include <SDL3/SDL_video.h>
#include "bhConfig.hpp"

int bhConfig::Load(const char* path, bhConfig& cfg)
{
	FILE* file = NULL;
	int result = fopen_s(&file, path, "rb");
	if (result == 0)
	{
		fread(&cfg, sizeof(bhConfig), 1, file);
		fclose(file);
	}
	return result;
}

int bhConfig::Save(const char* path, const bhConfig& cfg)
{
	FILE* file = NULL;
	int result = fopen_s(&file, path, "wb");
	if (result == 0)
	{
		fwrite(&cfg, sizeof(bhConfig), 1, file);
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
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, config.window.h);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, config.window.w);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
	}
	return props;
}

void bhConfig::DestroyProperties(SDL_PropertiesID props)
{
	SDL_DestroyProperties(props);
}
