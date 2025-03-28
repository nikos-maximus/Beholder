#include "bhMemory.h"
#include "bhDefines.h"
#include <inttypes.h>
#include <assert.h>

#define bhMemSz_t int32_t //int64_t

namespace bhMemory
{
    static bhMemSz_t g_totalMemory = 0;

    bool Init()
    {
        bhMemSz_t bitsAvailableForMB = (sizeof(bhMemSz_t) << 3) - 20;
        // bhMemSz_t has a = sizeof(bhMemSz_t) bytes -> sizeof(bhMemSz_t) * 8 bits available
        // For a value of m megabytes, we need (m * 2^20) bytes
        // hence a - 20 bits available for the value in MB
        assert(BH_MAX_MEMORY_BUDGET_MB < (1 << bitsAvailableForMB));
        return true;
    }

    void Destroy()
    {}
}
