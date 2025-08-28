#include "vulkanCore.hpp"
#include <set>
#include <string>
#include <algorithm>

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
  } else {
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
}
