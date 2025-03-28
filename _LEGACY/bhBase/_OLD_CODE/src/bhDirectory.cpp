#include "bhDirectory.h"

#ifdef _WINDOWS
#include <dirent.h>
#else
#include <dirent.h>
#endif

////////////////////////////////////////////////////////////////////////////////
Directory::Entry::Entry()
	: _name(nullptr)
{}

Directory::Entry::Entry(char const* iName)
{}

Directory::Entry::~Entry()
{
	delete[] _name;
	_name = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
Directory::Directory()
	: _path(nullptr)
{}

Directory::Directory(char const* path)
{
	_path = new char[strlen(path) + 1];
	strcpy(_path, path);
}

Directory::~Directory()
{
	delete[] _path;
	_path = nullptr;
}

bool Directory::Scan()
{
	DIR* dir = opendir(_path);
	if (dir)
	{
		dirent* entry = nullptr;
		while (entry = readdir(dir))
		{
			if (entry->d_type == DT_DIR)
			{
				continue;
			}
			_entries_v.push_back(entry->d_name);
		}
		closedir(dir);
		return false;
	}
	return false;
}
