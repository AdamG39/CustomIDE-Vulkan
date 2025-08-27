#include "buffer.hpp"
#include "../helpers/errors/errors.hpp"
#include <cstring>

void VulkanBuffer::Update(VkDevice Device, const void* pData, size_t Size) {
  void* pMemory = NULL;
  if (vkMapMemory(Device, memory, 0, Size, 0, &pMemory) != VK_SUCCESS)
    ExitWithError("Failed to map memory for buffer", -17);
  memcpy(pMemory, pData, Size);
  vkUnmapMemory(Device, memory);
}

void VulkanBuffer::Destroy(VkDevice Device) {
  if (buffer) {
    vkDestroyBuffer(Device, buffer, nullptr);
  }

  if (memory) {
    vkFreeMemory(Device, memory, nullptr);
  }
}

