#include "vulkanCore.hpp"
#include "texture.hpp"
#include "buffer.hpp"
#include "../helpers/errors/errors.hpp"
#include "../io/io.hpp"

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

void CreateTexture(const char* pFilename, VulkanTexture& Texture,
                   const VkDevice& Device, const VkPhysicalDevice& PhysicalDevice,
                   const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                   const VkQueue& GraphicsQueue) {
  std::vector<std::shared_ptr<Image>> image;
  if (!ReadImageFile(pFilename, image)) ExitWithError("Failed to load file", -11);

  VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
  CreateTextureImageFromData(Texture, image[0]->pixels, image[0]->width, image[0]->height, format,
                             Device, PhysicalDevice, CommandBuffers, CommandBufferIndex, GraphicsQueue);

  Texture.view = CreateImageView(Device, Texture.image, format, VK_IMAGE_ASPECT_COLOR_BIT);

  VkFilter minFilter = VK_FILTER_LINEAR;
  VkFilter maxFilter = VK_FILTER_LINEAR;
  VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  Texture.sampler = CreateTextureSampler(Device, minFilter, maxFilter, addressMode);
}


void CreateTextureImageFromData(VulkanTexture& Texture, const void* pPixels, uint32_t ImageWidth,
                                uint32_t ImageHeight, const VkFormat& TextureFormat,
                                const VkDevice& Device, const VkPhysicalDevice& PhysicalDevice,
                                const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                                const VkQueue& GraphicsQueue) {
  VkImageUsageFlagBits usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                      VK_IMAGE_USAGE_SAMPLED_BIT);
  VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  CreateTextureImage(Texture, ImageWidth, ImageHeight, TextureFormat, usage, propertyFlags, Device, PhysicalDevice);

  UpdateTextureImage(Texture, ImageWidth, ImageHeight, TextureFormat, pPixels,
                     Device, PhysicalDevice, CommandBuffers, CommandBufferIndex, GraphicsQueue);
}

void CreateTextureImage(VulkanTexture& Texture, uint32_t ImageWidth, uint32_t ImageHeight,
                        const VkFormat& TextureFormat, const VkImageUsageFlags& UsageFlags,
                        const VkMemoryPropertyFlags& PropertyFlags, const VkDevice& Device,
                        const VkPhysicalDevice& PhysicalDevice) {
  VkImageCreateInfo imageInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = TextureFormat,
    .extent = VkExtent3D { .width = ImageWidth, .height = ImageHeight, .depth = 1 },
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = UsageFlags,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = nullptr,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
  };

  if (vkCreateImage(Device, &imageInfo, NULL, &Texture.image) != VK_SUCCESS)
      ExitWithError("Failed to create image", -12);

  VkMemoryRequirements memoryRequirements = { 0 };
  vkGetImageMemoryRequirements(Device, Texture.image, &memoryRequirements);

  uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, PropertyFlags, PhysicalDevice);

  VkMemoryAllocateInfo memoryAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = nullptr,
    .allocationSize = memoryRequirements.size,
    .memoryTypeIndex = memoryTypeIndex
  };

  if (vkAllocateMemory(Device, &memoryAllocateInfo, nullptr, &Texture.memory) != VK_SUCCESS)
    ExitWithError("Failed to allocate memory for image", -14);

  if (vkBindImageMemory(Device, Texture.image, Texture.memory, 0) != VK_SUCCESS)
    ExitWithError("Failed to bind image memory", -15);
}

void UpdateTextureImage(VulkanTexture& Texture, uint32_t ImageWidth, uint32_t ImageHeight,
                        const VkFormat& TextureFormat, const void* pPixels,
                        const VkDevice& Device, const VkPhysicalDevice& PhysicalDevice,
                        const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                        const VkQueue& GraphicsQueue) {
  int bytesPerPixel = GetBytesPerTextureFormat(TextureFormat);

  VkDeviceSize layerSize = ImageWidth * ImageHeight * bytesPerPixel;
  int layerCount = 1;
  VkDeviceSize imageSize = layerCount * layerSize;

  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  VulkanBuffer stagingBuffer = CreateBuffer(imageSize, usage, properties, Device, PhysicalDevice);

  stagingBuffer.Update(Device, pPixels, imageSize);

  TransitionImageLayout(Texture.image, TextureFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, CommandBuffers,
                        CommandBufferIndex, GraphicsQueue);

  CopyBufferToImage(Texture.image, stagingBuffer.buffer, ImageWidth, ImageHeight,
                    CommandBuffers, CommandBufferIndex, GraphicsQueue);

  TransitionImageLayout(Texture.image, TextureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, CommandBuffers,
                        CommandBufferIndex, GraphicsQueue);

  stagingBuffer.Destroy(Device);
}

void TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout,
                           const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                           const VkQueue& GraphicsQueue) {
  BeginCommandBuffer(CommandBuffers[CommandBufferIndex], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  ImageMemoryBarrier(CommandBuffers[CommandBufferIndex], Image, Format, OldLayout, NewLayout);

  SubmitCopyCommand(CommandBuffers, CommandBufferIndex, GraphicsQueue);
}

int GetBytesPerTextureFormat(const VkFormat& TextureFormat) {
  switch (TextureFormat) {
  case VK_FORMAT_R8_SINT:
  case VK_FORMAT_R8_UNORM:
    return 1;
  case VK_FORMAT_R16_SFLOAT:
    return 2;
  case VK_FORMAT_R16G16_SFLOAT:
  case VK_FORMAT_R16G16_SNORM:
  case VK_FORMAT_B8G8R8A8_UNORM:
  case VK_FORMAT_R8G8B8A8_UNORM:
    return 4;
  case VK_FORMAT_R16G16B16A16_SFLOAT:
    return 4 * sizeof(uint16_t);
  case VK_FORMAT_R32G32B32A32_SFLOAT:
    return 4 * sizeof(float);
  default:
    ExitWithError("Not support format", -16);
  }

  return 0;
}

void CopyBufferToImage(const VkImage& Destination, VkBuffer Source, uint32_t ImageWidth,
                       uint32_t ImageHeight, const VkCommandBuffer* CommandBuffers,
                       uint32_t CommandBufferIndex, const VkQueue& GraphicsQueue) {
  BeginCommandBuffer(CommandBuffers[CommandBufferIndex], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferImageCopy BufferImageCopy = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = VkImageSubresourceLayers {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.imageOffset = VkOffset3D {.x = 0, .y = 0, .z = 0 },
		.imageExtent = VkExtent3D {.width = ImageWidth, .height = ImageHeight, .depth = 1 }
	};

	vkCmdCopyBufferToImage(CommandBuffers[CommandBufferIndex], Source, Destination,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);

	SubmitCopyCommand(CommandBuffers, CommandBufferIndex, GraphicsQueue);
}

void ImageMemoryBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format,
                        VkImageLayout OldLayout, VkImageLayout NewLayout) {
  VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = OldLayout,
		.newLayout = NewLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = Image,
		.subresourceRange = VkImageSubresourceRange {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

	if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
		(Format == VK_FORMAT_D16_UNORM) ||
		(Format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
		(Format == VK_FORMAT_D32_SFLOAT) ||
		(Format == VK_FORMAT_S8_UINT) ||
		(Format == VK_FORMAT_D16_UNORM_S8_UINT) ||
		(Format == VK_FORMAT_D24_UNORM_S8_UINT))
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (Format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        Format == VK_FORMAT_D24_UNORM_S8_UINT) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_GENERAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
		NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} /* Convert back from read-only to updateable */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} /* Convert from updateable texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
		     NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} /* Convert depth texture from undefined state to depth-stencil buffer */
	else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} /* Wait for render pass to complete */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = 0;
		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} /* Convert back from read-only to color attachment */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} /* Convert from updateable texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} /* Convert back from read-only to depth attachment */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	} /* Convert from updateable depth texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} 
	else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	else {
		ExitWithError("Unknown barrier case", 1);
	}

	vkCmdPipelineBarrier(CmdBuf, sourceStage, destinationStage, 
		                 0, 0, NULL, 0, NULL, 1, &barrier);
}

