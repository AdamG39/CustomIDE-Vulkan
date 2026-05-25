#include "vulkanCore.hpp"
#include "../helpers/errors/errors.hpp"
#include <set>
#include <string>
#include <algorithm>
#include <cstring>
#include <limits>

bool CheckValidationLayerSupport(const char* const* ValidationLayers, size_t LayersSize) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (size_t i = 0; i < LayersSize; i++) {
    bool layerFound = false;

    for (const auto& layerProperties: availableLayers) {
      if (strcmp(ValidationLayers[i], layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

// Checks if the supplied GPU is suitable for this program
bool IsDeviceSuitable(VkPhysicalDevice Device, VkSurfaceKHR Surface,
                      const std::vector<const char*>& DeviceExtensions) {
  QueueFamilyIndicies indicies = FindQueueFamilies(Device, Surface);

  bool extensionsSupported = CheckDeviceExtensionSupport(Device, DeviceExtensions.data(), DeviceExtensions.size());

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(Device, Surface);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  return indicies.IsComplete() && extensionsSupported && swapChainAdequate;
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice Device, const char* const* DeviceExtensions,
                                 size_t ExtensionsCount) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions;
  for (size_t i = 0; i < ExtensionsCount; i++) {
    requiredExtensions.insert(DeviceExtensions[i]);
  }

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

// Find all queue families and check for compatabilities
QueueFamilyIndicies FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface) {
  QueueFamilyIndicies indicies;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, queueFamilies.data());

  // Loop through all available queue families and if suitable save its index
  int i = 0;
  VkBool32 presentSupport = false;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indicies.graphicsFamily = i;
    }

    vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Surface, &presentSupport);

    if (presentSupport) {
      indicies.presentFamily = i;
    }

    if (indicies.IsComplete()) {
      break;
    }

    i++;
  }

  return indicies;
}

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats) {
  for (const auto& availableFormat : AvailableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return AvailableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes) {
  for (const auto& availablePresentMode : AvailablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, GLFWwindow* Window) {
  if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return Capabilities.currentExtent;
  } 

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(Window, &width, &height);

  VkExtent2D actualExtent = {
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height)
  };

  actualExtent.width = std::clamp(actualExtent.width, 
      Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
  actualExtent.height = std::clamp(actualExtent.height, 
      Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

  return actualExtent;
}

void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags) {
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = UsageFlags,
		.pInheritanceInfo = NULL
	};

  if (vkBeginCommandBuffer(CommandBuffer, &beginInfo) != VK_SUCCESS)
    ExitWithError("Failed to start command buffer", -19);
}

void SubmitCopyCommand(const VkCommandBuffer* CommandBuffers, uint32_t CommandBufferIndex,
                       const VkQueue& GraphicsQueue) {
  vkEndCommandBuffer(CommandBuffers[CommandBufferIndex]);

  VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = VK_NULL_HANDLE,
		.pWaitDstStageMask = VK_NULL_HANDLE,
		.commandBufferCount = 1,
		.pCommandBuffers = &CommandBuffers[CommandBufferIndex],
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = VK_NULL_HANDLE
	};

	if (vkQueueSubmit(GraphicsQueue, 1, &submitInfo, NULL) != VK_SUCCESS)
    ExitWithError("Failed to submit queue", 1);

  vkQueueWaitIdle(GraphicsQueue);
}

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format,
                            VkImageAspectFlags AspectFlags) {
  VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = Image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = Format,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange = {
			.aspectMask = AspectFlags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	VkImageView ImageView;
	if (vkCreateImageView(Device, &viewInfo, NULL, &ImageView) != VK_SUCCESS)
    ExitWithError("Failed to create image view", 1);
	return ImageView;
}

VkSampler CreateTextureSampler(const VkDevice& Device, const VkFilter& MinFilter,
                               const VkFilter& MaxFilter, const VkSamplerAddressMode& AddressMode) {
  VkSamplerCreateInfo samplerInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.magFilter = MinFilter,
		.minFilter = MaxFilter,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = AddressMode,
		.addressModeV = AddressMode,
		.addressModeW = AddressMode,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};

	VkSampler Sampler;
	if (vkCreateSampler(Device, &samplerInfo, VK_NULL_HANDLE, &Sampler) != VK_SUCCESS)
    ExitWithError("Failed to create sampler", 1);
	return Sampler;
}

VkRenderPass CreateRenderPass(const VkFormat& SwapChainFormat, const VkDevice& Device) {
  VkAttachmentDescription colourAttachment{};
  colourAttachment.format = SwapChainFormat;
  colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colourAttachmentRef{};
  colourAttachmentRef.attachment = 0;
  colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colourAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colourAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VkRenderPass renderPass;

  if (vkCreateRenderPass(Device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    ExitWithError("Failed to create render pass!", -1);
  }

  return renderPass;
}

VkShaderModule CreateShaderModule(const char* pCodeData, size_t CodeSize, const VkDevice& Device) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = CodeSize;
  createInfo.pCode = reinterpret_cast<const uint32_t*>(pCodeData);

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    ExitWithError("Failed to create shader module", -1);
  }

  return shaderModule;
}

std::string GetFileNameFromPath(const std::string& filePath, bool includeExtension) {
  size_t forwardSlash = filePath.find_last_of('/');
  size_t backSlash = filePath.find_last_of('\\');

  std::string result;

  size_t slashSeperatorPos;
  if (forwardSlash == std::string::npos &&
        backSlash == std::string::npos) {
    slashSeperatorPos = -1;
  }
  else {
    if (forwardSlash == std::string::npos)
      slashSeperatorPos = backSlash;
    else if (backSlash == std::string::npos)
      slashSeperatorPos = forwardSlash;
    else
      slashSeperatorPos = (forwardSlash > backSlash) ? forwardSlash : backSlash;
  }

  result = filePath.substr(slashSeperatorPos + 1);

  if (!includeExtension) {
    size_t extensionStart = result.find_last_of('.');

    if (extensionStart != std::string::npos)
      return result.substr(0, extensionStart);

    return result;
  }

  return result;
}

