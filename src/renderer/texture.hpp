#ifndef CUSTOM_VULKAN_TEXTURE_H
#define CUSTOM_VULKAN_TEXTURE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "shapes.hpp"

namespace Vulkan {

struct ImageMetadata {
  CustomIDE::Vector2<int> dimensions;
};

struct Texture {
  ImageMetadata metadata {};
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;

  Texture() = default;

  void Destroy(VkDevice Device);
};

class TextureBufferMap {
private:
  std::vector<Texture> m_textures;
  size_t m_maxCapacity{};
  size_t m_size{};
  std::vector<std::string> m_fileNames;

public:
  TextureBufferMap(size_t MaxTextures);

  const Texture& operator[](size_t Index) const;
  Texture& operator[](std::string FileName);

  size_t Insert(std::string FileName, Texture Texture = {});

  size_t Capacity() const { return m_maxCapacity; }

  size_t Size() const { return m_size; }

  size_t Contains(std::string FileName) const;

  void Destroy(VkDevice Device);
};

void CreateTexture(const char* pFilename, Texture& Texture,
                   const VkDevice& Device, const VkPhysicalDevice& PhysicalDevice,
                   const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                   const VkQueue& GraphicsQueue);

void CreateTextureImageFromData(Texture& Texture, const void* pPixels, uint32_t ImageWidth,
                                uint32_t ImageHeight, const VkFormat& TextureFormat,
                                const VkDevice& Device, const VkPhysicalDevice& PhysicalDevice,
                                const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                                const VkQueue& GraphicsQueue);

void CreateTextureImage(Texture& Texture, uint32_t ImageWidth, uint32_t ImageHeight,
                        const VkFormat& TextureFormat, const VkImageUsageFlags& UsageFlags,
                        const VkMemoryPropertyFlags& PropertyFlags, const VkDevice& Device,
                        const VkPhysicalDevice& PhysicalDevice);

void UpdateTextureImage(Texture& Texture, uint32_t ImageWidth, uint32_t ImageHeight,
                        const VkFormat& TextureFormat, const void* pPixels,
                        const VkDevice& Device, const VkPhysicalDevice& PhysicalDevice,
                        const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                        const VkQueue& GraphicsQueue);

void TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout,
                           const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                           const VkQueue& GraphicsQueue);

int GetBytesPerTextureFormat(const VkFormat& TextureFormat);

void CopyBufferToImage(const VkImage& Destination, VkBuffer Source, uint32_t ImageWidth,
                       uint32_t ImageHeight, const VkCommandBuffer* CommandBuffers,
                       uint32_t CommandBufferIndex, const VkQueue& GraphicsQueue);

void ImageMemoryBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format,
                        VkImageLayout OldLayout, VkImageLayout NewLayout);

} // namespace Vulkan

#endif

