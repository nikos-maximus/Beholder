#ifndef BH_RESOURCE_CACHE_COMMON_HPP
#define BH_RESOURCE_CACHE_COMMON_HPP

#include <map>
#include <physfs.h>
#include "bhResource.hpp"
#include "bhHash.hpp"

template<class Resource_T>
void bhResource_AddDirectory(std::map<bhHash_t, bhResource<Resource_T>>& outRes, const std::string& dir)
{
	char** files = PHYSFS_enumerateFiles(dir.c_str());
	if (!files)
	{
		return;
	}
	char path[BH_PATH_BUF_LEN];
	int fi = 0;
	while (files[fi])
	{
		sprintf_s(path, BH_PATH_BUF_LEN, "%s/%s", dir.c_str(), files[fi]);
		if (PHYSFS_isDirectory(path))
		{
			bhResource_AddDirectory(outRes, path);
		}
		else
		{
			bhResource<Resource_T> newRsrc(path);
			outRes[bhHash(path)] = std::move(newRsrc);
		}
		++fi;
	}
	PHYSFS_freeList(files);
}

#endif //BH_RESOURCE_CACHE_COMMON_HPP
