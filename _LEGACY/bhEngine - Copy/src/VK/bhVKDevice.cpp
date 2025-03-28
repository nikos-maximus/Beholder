#include <assert.h>
#include "VK/bhVKDevice.hpp"
#include "bhLog.h"

bhVK::Buffer bhVKDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize reqSize) const
{
  VkBufferCreateInfo bufferCreateInfo =
  {
    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0,
    reqSize,
    usageFlags,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr
  };

  bhVK::Buffer newBuffer;
  if (vkCreateBuffer(device, &bufferCreateInfo, allocCB, &(newBuffer.buffer)) != VK_SUCCESS)
  {
    // TODO: Report error
    return newBuffer;
  }
  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device, newBuffer.buffer, &memoryRequirements);

  //VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  uint32_t memTypeIdx = bhVK::FindPhysicalDeviceMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, propertyFlags);
  if (memTypeIdx < UINT32_MAX)
  {
    VkMemoryAllocateInfo memoryAllocateInfo =
    {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
      memoryRequirements.size,
      memTypeIdx
    };

    if (vkAllocateMemory(device, &memoryAllocateInfo, allocCB, &(newBuffer.memory)) == VK_SUCCESS)
    {
      newBuffer.size = memoryRequirements.size;
      vkBindBufferMemory(device, newBuffer.buffer, newBuffer.memory, 0); // TODO: Error check
      return newBuffer;
    }
  }

  DestroyBuffer(newBuffer);
  return newBuffer;
}

VkBool32 bhVKDevice::CopyDataToBuffer(const bhVK::Buffer& buffer, VkDeviceSize offset, VkDeviceSize reqSize, const void* data) const
{
  assert(buffer.memory != VK_NULL_HANDLE);
  if (offset + reqSize <= buffer.size)
  {
    void* memory = nullptr;
    if (vkMapMemory(device, buffer.memory, offset, reqSize, 0, &memory) == VK_SUCCESS)
    {
      memcpy(memory, data, reqSize);
      //{
      //	VkMappedMemoryRange mmr = {};
      //	mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      //	mmr.memory = deviceMemory;
      //	mmr.offset = offset;
      //	mmr.size = reqSize;
      //	vkFlushMappedMemoryRanges(device, 1, &mmr);
      //}
      vkUnmapMemory(device, buffer.memory);
      return VK_TRUE;
    }
  }
  bhLog_Message(LP_CRITICAL, "CopyDataToBuffer : Exceeding buffer size!");
  return VK_FALSE;
}

void bhVKDevice::DestroyBuffer(bhVK::Buffer& buffer) const
{
  vkFreeMemory(device, buffer.memory, allocCB);
  vkDestroyBuffer(device, buffer.buffer, allocCB);
  buffer = {};
}
