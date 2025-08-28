#ifndef CUSTOM_VULKAN_CORE_H
#define CUSTOM_VULKAN_CORE_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>

struct QueueFamilyIndicies {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool IsComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct PushConstants {
  float width;
  float height;
};

bool CheckValidationLayerSupport(const char* const* ValidationLayers, size_t LayersSize);

bool IsDeviceSuitable(VkPhysicalDevice Device, VkSurfaceKHR Surface,
                      const std::vector<const char*>& DeviceExtensions);

bool CheckDeviceExtensionSupport(VkPhysicalDevice Device, const char* const* DeviceExtensions,
                                 size_t ExtensionsCount);

QueueFamilyIndicies FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface);

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device, VkSurfaceKHR Surface);

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes);

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, GLFWwindow* Window);

void PickPhysicalDevice(VkInstance Instance, VkPhysicalDevice& PhysicalDevice, VkSurfaceKHR Surface,
                        std::vector<const char*>& DeviceExtensions) ;

VkImageView CreateImageView(VkDevice, VkImage Image, VkFormat Format,
                            VkImageAspectFlags AspectFlags);

VkSampler CreateTextureSampler(VkDevice Device, VkFilter MinFilter, VkFilter MaxFilter, 
                               VkSamplerAddressMode AddressMode);

void CloseWindowCallback(GLFWwindow* window);

void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods);

void FramebufferResizeCallback(GLFWwindow* Window, int Width, int Height);

void CursorPositionCallback(GLFWwindow* Window, double xpos, double ypos);

void ToggleMaximiseCallback(GLFWwindow* Window);

void MinimiseCallback(GLFWwindow* Window);

#endif

