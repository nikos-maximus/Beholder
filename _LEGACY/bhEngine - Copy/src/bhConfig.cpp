#include <stdio.h>
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
