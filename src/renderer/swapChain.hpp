#ifndef CUSTOM_VULKAN_SWAPCHAIN_H
#define CUSTOM_VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace Vulkan {

class SwapChain {
private:
  GLFWwindow* m_window;
  std::shared_ptr<VkDevice> m_device;
  std::shared_ptr<VkPhysicalDevice> m_physicalDevice;
  std::shared_ptr<VkSurfaceKHR> m_surface;
  uint32_t m_imageCount;

  VkSwapchainKHR m_swapChain;
  std::vector<VkImage> m_swapChainImages;
  VkFormat m_swapChainImageFormat;
  VkExtent2D m_swapChainExtent;
  std::vector<VkImageView> m_swapChainImageViews;
  std::vector<VkFramebuffer> m_swapChainFramebuffers;

public:
  SwapChain(GLFWwindow* Window, const VkDevice& Device,
            const VkPhysicalDevice& PhysicalDevice, const VkSurfaceKHR& Surface)
  : m_window(Window),
    m_device(std::make_shared<VkDevice>(Device)),
    m_physicalDevice(std::make_shared<VkPhysicalDevice>(PhysicalDevice)),
    m_surface(std::make_shared<VkSurfaceKHR>(Surface)) {}

  ~SwapChain() {
    for (auto framebuffer: m_swapChainFramebuffers) {
      vkDestroyFramebuffer(*m_device, framebuffer, nullptr);
    }

    for (auto imageView: m_swapChainImageViews) {
      vkDestroyImageView(*m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(*m_device, m_swapChain, nullptr);
  }

  void CreateSwapChain();
  
  void CreateImageViews();

  void CreateFramebuffers(VkRenderPass RenderPass);

  void RecreateSwapChain(VkRenderPass RenderPass);

  VkSwapchainKHR GetSwapChain() const { return m_swapChain; }
  std::vector<VkImage> GetImages() const { return m_swapChainImages; }
  VkFormat GetImageFormat() const { return m_swapChainImageFormat; }
  VkExtent2D GetExtent() const { return m_swapChainExtent; }
  std::vector<VkImageView> GetImageViews() const { return m_swapChainImageViews; }
  std::vector<VkFramebuffer> GetFramebuffers() const { return m_swapChainFramebuffers; }
  uint32_t GetImageCount() const { return m_imageCount; }
};

} // namespace Vulkan

#endif

