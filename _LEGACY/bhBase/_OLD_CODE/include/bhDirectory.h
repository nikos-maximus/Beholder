#pragma once
#include <vector>

class Directory
{
	////////////////////////////////////////////////////////////////////////////////
	struct Entry
	{
		Entry();
		Entry(char const* name);
		~Entry();

		char const* _name;
	};

private:

	Directory();

	std::vector<Entry> _entries_v;
	char* _path;

public:

	Directory(char const* path);
	~Directory();

	bool Scan();

protected:
};
