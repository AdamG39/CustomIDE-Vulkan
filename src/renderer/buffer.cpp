#include "buffer.hpp"
#include "../helpers/errors/errors.hpp"
#include <cstring>

void Vulkan::Buffer::Update(VkDevice Device, const void* pData, size_t Size) {
  void* pMemory = NULL;
  if (vkMapMemory(Device, memory, 0, Size, 0, &pMemory) != VK_SUCCESS)
    CustomIDE::Errors::ExitWithError("Failed to map memory for buffer", -17);
  memcpy(pMemory, pData, Size);
  vkUnmapMemory(Device, memory);
}

void Vulkan::Buffer::Destroy(VkDevice Device) {
  if (buffer) {
    vkDestroyBuffer(Device, buffer, nullptr);
  }

  if (memory) {
    vkFreeMemory(Device, memory, nullptr);
  }
}

Vulkan::Buffer Vulkan::CreateBuffer(const VkDeviceSize& Size, const VkBufferUsageFlags& Usage,
                                    const VkMemoryPropertyFlags& Properties, const VkDevice& Device,
                                    const VkPhysicalDevice& PhysicalDevice) {
  VkBufferCreateInfo bufferInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = Size,
    .usage = Usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  Buffer buffer;

  if (vkCreateBuffer(Device, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
    CustomIDE::Errors::ExitWithError("Failed to create buffer", -18);

  VkMemoryRequirements memoryRequirements = { 0 };
  vkGetBufferMemoryRequirements(Device, buffer.buffer, &memoryRequirements);

  buffer.allocationSize = memoryRequirements.size;

  uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, Properties, PhysicalDevice);

  VkMemoryAllocateInfo memoryAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = nullptr,
    .allocationSize = memoryRequirements.size,
    .memoryTypeIndex = memoryTypeIndex
  };

  if (vkAllocateMemory(Device, &memoryAllocateInfo, nullptr, &buffer.memory) != VK_SUCCESS)
    CustomIDE::Errors::ExitWithError("Failed to allocate memory for image", -14);

  if (vkBindBufferMemory(Device, buffer.buffer, buffer.memory, 0) != VK_SUCCESS)
    CustomIDE::Errors::ExitWithError("Failed to bind image memory", -15);

  return buffer;
}

uint32_t Vulkan::GetMemoryTypeIndex(uint32_t MemoryTypeBitsMask, const VkMemoryPropertyFlags& RequiredMemoryPropertyFlags,
                            const VkPhysicalDevice& PhysicalDevice) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memoryProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    VkMemoryType memoryType = memoryProperties.memoryTypes[i];
    uint32_t currentBitmask = (1 << i);
    bool isCurrentMemoryTypeSupported = (MemoryTypeBitsMask & currentBitmask);
    bool hasRequiredMemoryProperties = 
      ((memoryType.propertyFlags & RequiredMemoryPropertyFlags) == RequiredMemoryPropertyFlags);

    if (isCurrentMemoryTypeSupported && hasRequiredMemoryProperties) return i;
  }

  CustomIDE::Errors::ExitWithError("No valid memory type found", -13);
  return -1;
}

