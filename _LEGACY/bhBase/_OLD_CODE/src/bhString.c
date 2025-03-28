#include "bhString.h"
#include <stdlib.h>

size_t bhStrlen(const char* str)
{
    size_t len = 0;
    while (str++ != '\0')
    {
        ++len;
    }
    return len;
}

void bhString_Delete(bhString* str)
{
    free(str->str);
    str->str = NULL;
    str->size = 0;
}

bhString bhString_Init()
{
    bhString s;
    s.str = NULL;
    s.size = 0;
    return s;
}

bhString bhString_InitFromPChar(const char* str)
{
    bhString s;
    s.size = bhStrlen(str);
    s.str = calloc(s.size, sizeof(char));
    for (size_t sz = 0; sz < s.size; ++sz)
    {
        s.str[sz] = str[sz];
    }
    return s;
}
