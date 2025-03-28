#include "../include/bhAssetManager.h"
#include "../include/bhString.h"
#include <assert.h>

#ifdef _WINDOWS
#include <dirent/include/dirent.h>
#else
#include <dirent.h>
#endif

////////////////////////////////////////////////////////////////////////////////

bhAssetManager::Entry::Entry()
	: _name(nullptr)
{}

bhAssetManager::Entry::Entry(char const* name)
{
	_name = bhString::Copy(name);
}

bhAssetManager::Entry::~Entry()
{
	delete[] _name;
	_name = nullptr;
}

int bhAssetManager::Entry::Compare(const void* first, const void* second)
{
	return strcmp(((Entry*)first)->_name, ((Entry*)second)->_name);
}

////////////////////////////////////////////////////////////////////////////////

bhAssetManager::bhAssetManager()
	: _baseDirName(nullptr)
{}

bhAssetManager::bhAssetManager(char const* baseDir)
{
	// baseDir is meant to be assigned from bhEnv::GetEnvString. No copy needed
	_baseDirName = baseDir;
}

void bhAssetManager::Destroy(AssetDestroyFunc)
{
	for (size_t a = 0; a < _assets.size(); ++a)
	{
		bhAsset* ptr = _assets[a];
		AssetDestroyFunc(ptr);
		delete ptr;
		ptr = nullptr;
	}
}

bool bhAssetManager::AddDirectory(char const* dir)
{
	for (size_t d = 0; d < _dirNames.size(); ++d)
	{
		if (strcmp(_dirNames[d]._name, dir) == 0)
		{
			return false;
		}
	}
	_dirNames.push_back(dir);
	return true;
}

bool bhAssetManager::ScanDirEntries(char const* name)
{
	DIR* dir = opendir(name);
	if (!dir)
	{
		return false;
	}
	dirent* entry = readdir(dir);
	while (entry)
	{
		if (S_ISDIR(entry->d_type)) // non-recursive
		{
			continue;
		}
		_entries.push_back(name);
		entry = readdir(dir);
	}
	closedir(dir);
	return true;
}

bool bhAssetManager::ScanEntries()
{
	assert(_baseDirName != nullptr);
	if (ScanDirEntries(_baseDirName))
	{
		for (size_t d = 0; d < _dirNames.size(); ++d)
		{
			char const* currDirName = _dirNames[d]._name;
			currDirName = bhString::Append(_dirNames[0]._name, _dirNames[d]._name);
			ScanDirEntries(currDirName);
		}
		std::qsort(_entries.data(), _entries.size(), sizeof(Entry), Entry::Compare);
		_assets.resize(_entries.size());
		return true;
	}
	return false;
}

bool bhAssetManager::Find(char const* path, size_t* out_index)
{
	// TODO: apply a faster search
	for (size_t e = 0; e < _entries.size(); ++e)
	{
		if (strcmp(path, _entries[e]._name) == 0)
		{
			*out_index = e;
			return true;
		}
	}
	return false;
}

bool bhAssetManager::AddAsset(bhAsset* asset, size_t index)
{
	if ((asset != nullptr) && (index < _entries.size()))
	{
		_assets[index] = asset;
		return true;
	}
	return false;
}
