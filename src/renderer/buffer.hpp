#ifndef CUSTOM_VULKAN_BUFFER_H
#define CUSTOM_VULKAN_BUFFER_H

#include <vulkan/vulkan.h>

struct VulkanBuffer {
  VkBuffer buffer = NULL;
  VkDeviceMemory memory = NULL;
  VkDeviceSize allocationSize = 0;

  VulkanBuffer() {}

  void Update(VkDevice Device, const void* pData, size_t Size);

  void Destroy(VkDevice Device);
};

VulkanBuffer CreateBuffer(const VkDeviceSize& Size, const VkBufferUsageFlags& Usage,
                          const VkMemoryPropertyFlags& Properties, const VkDevice& Device,
                          const VkPhysicalDevice& PhysicalDevice);

uint32_t GetMemoryTypeIndex(uint32_t MemoryTypeBitsMask, const VkMemoryPropertyFlags& RequiredMemoryPropertyFlags,
                            const VkPhysicalDevice& PhysicalDevice);

#endif

