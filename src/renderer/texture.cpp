#include "texture.hpp"

void VulkanTexture::Destroy(VkDevice Device) {
  if (image) {
    vkDestroyImage(Device, image, nullptr);
  }

  if (memory) {
    vkFreeMemory(Device, memory, nullptr);
  }

  if (view) {
    vkDestroyImageView(Device, view, nullptr);
  }

  if (sampler) {
    vkDestroySampler(Device, sampler, nullptr);
  }
}
