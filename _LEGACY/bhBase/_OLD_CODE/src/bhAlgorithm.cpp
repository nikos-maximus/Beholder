#include "bhAlgorithm.h"
#include <string.h>

namespace bhAlgorithm
{
	char const* RFind(char const* string, char c)
	{
		if (!string)
		{
			return nullptr;
		}
		for (size_t pos = strlen(string) - 1; pos >= 0; --pos)
		{
			if (string[pos] == c)
			{
				return &(string[pos]);
			}
		}
		return nullptr;
	}
}
