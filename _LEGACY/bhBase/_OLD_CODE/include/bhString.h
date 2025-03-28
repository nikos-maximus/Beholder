#ifndef BH_STRING_H
#define BH_STRING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    char* str;
    size_t size;
}
bhString;

size_t bhStrlen(const char* str);
void bhString_Delete(bhString* str);
bhString bhString_Init();
bhString bhString_InitFromPChar(const char* str);

#ifdef __cplusplus
}
#endif

#endif //BH_STRING_H
