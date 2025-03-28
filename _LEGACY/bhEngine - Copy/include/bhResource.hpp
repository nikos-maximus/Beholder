#ifndef BH_RESOURCE_HPP
#define BH_RESOURCE_HPP

#include <string>
#include <unordered_map>
#include <dirent.h>
#include "bhHash.hpp"
#include "bhUtil.hpp"
#include "Platform/bhPlatform.hpp"

////////////////////////////////////////////////////////////////////////////////
struct bhResourceInfo
{
  bhResourceInfo(const std::string& _name, const std::string& _path)
    : name(_name), path(_path)
  {}

  bhResourceInfo(const char* _name, const char* _path)
    : name(_name), path(_path)
  {}

  std::string name{ "INVALID_NAME" };
  std::string path{ "INVALID_PATH" };
};

void GenerateResourceHashes(std::unordered_map<bhHash_t, bhResourceInfo>& hashes, const char* dir, const char** extensions, const char* subDir = nullptr);

#endif //BH_RESOURCE_HPP
