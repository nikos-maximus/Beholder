#ifndef BH_HASH_HPP
#define BH_HASH_HPP

#include <stdint.h>
#include "bhDefines.h"

typedef size_t bhHash_t;

// Matched return type with def of size_t
// Assume stc C null-terminated str
//bhHash_t bhHash_Generic(char* str, uint32_t p, uint32_t m);
bhHash_t bhHash(char* str);
inline bhHash_t bhHash(const char* str) { return bhHash(const_cast<char*>(str)); }

#endif //BH_HASH_HPP
