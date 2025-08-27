#ifndef CUSTOM_VULKAN_TEXTURE_H
#define CUSTOM_VULKAN_TEXTURE_H

#include <vulkan/vulkan.h>

struct VulkanTexture {
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;

  VulkanTexture() {}

  void Destroy(VkDevice Device);
};

#endif

