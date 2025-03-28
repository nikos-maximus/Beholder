#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>
#include "bhHash.hpp"

/*
bhHash_t bhHash_Generic(char* str, uint32_t p, uint32_t m)
{
	//REF: https://cp-algorithms.com/string/string-hashing.html
	if (str == NULL)
	{
		return 0;
	}

	bhHash_t pow = 1;
	bhHash_t hash = 0;
	while (*str)
	{
		hash += pow * (*str);
		pow *= p;
		++str;
	}
	return hash % m;
}
*/

bhHash_t bhHash(char* str)
{
	// p = 53, m = 10^9 + 9
	return stbds_hash_string(str, 1000000009);
	//return bhHash_Generic(str, 53, 1000000009);
}
