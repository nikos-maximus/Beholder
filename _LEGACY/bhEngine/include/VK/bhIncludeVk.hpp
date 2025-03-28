#ifndef BH_INCLUDE_VK_HPP
#define BH_INCLUDE_VK_HPP

#if (_WIN32 || _WIN64)
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
//#elif // TODO: Other Platforms
#endif

#include <Volk/volk.h>

#define VMA_VULKAN_VERSION 1001000
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

#include "bhLog.h"

inline void CheckVkResult(VkResult result)
{
  if (result)
  {
    bhLog_Message(bhLogPriority::LP_ERROR, "[Vulkan] Error: %d\n", result);
    abort();
  }
}

#define BH_ASSERT_VK(expr)                                                  \
  do {                                                                      \
    VkResult rs = expr;                                                     \
    CheckVkResult(rs);                                                      \
  } while (0)

#endif //BH_INCLUDE_VK_HPP
