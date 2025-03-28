#include "bhTypes.hpp"

bhShaderType bhTypes_GetShaderTypeFromFileExtension(const char* path)
{
  static const char* extensions[NUM_SHADER_TYPES] =
  {
    "vert",
    "tesc",
    "tese",
    "geom",
    "frag",
    "comp",
  };

  const char* extension = strrchr(path, '.');
  if (extension && extension[1])
  {
    for (int i = 0; i < NUM_SHADER_TYPES; ++i)
    {
      if (strcmp(extensions[i], &(extension[1])) == 0)
      {
        return static_cast<bhShaderType>(i);
      }
    }
  }
  return SHADER_TYPE_UNKNOWN;
}
