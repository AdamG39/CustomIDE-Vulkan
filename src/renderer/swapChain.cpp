#include "swapChain.hpp"
#include "vulkanCore.hpp"
#include "../helpers/errors/errors.hpp"

void SwapChain::RecreateSwapChain(VkRenderPass RenderPass) {
  if (glfwWindowShouldClose(m_window)) { return; }
  vkDeviceWaitIdle(*m_device);

  for (auto framebuffer: m_swapChainFramebuffers) {
    vkDestroyFramebuffer(*m_device, framebuffer, nullptr);
  }
  m_swapChainFramebuffers.clear();

  for (auto imageView: m_swapChainImageViews) {
    vkDestroyImageView(*m_device, imageView, nullptr);
  }
  m_swapChainImageViews.clear();

  vkDestroySwapchainKHR(*m_device, m_swapChain, nullptr);

  CreateSwapChain();
  CreateImageViews();
  CreateFramebuffers(RenderPass);
}

void SwapChain::CreateSwapChain() {
  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(*m_physicalDevice, *m_surface);

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, m_window);

  m_imageCount = swapChainSupport.capabilities.minImageCount;

  if (swapChainSupport.capabilities.maxImageCount > 0 && m_imageCount > swapChainSupport.capabilities.maxImageCount) {
    m_imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = *m_surface;

  createInfo.minImageCount = m_imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndicies indicies = FindQueueFamilies(*m_physicalDevice, *m_surface);
  uint32_t queueFamilyIndicies[] = {indicies.graphicsFamily.value(), indicies.presentFamily.value()};

  if (indicies.graphicsFamily != indicies.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndicies;
  } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0; // Optional
      createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(*m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
    ExitWithError("Failed to create swap chain!", -1);
  }

  vkGetSwapchainImagesKHR(*m_device, m_swapChain, &m_imageCount, nullptr);
  m_swapChainImages.resize(m_imageCount);
  vkGetSwapchainImagesKHR(*m_device, m_swapChain, &m_imageCount, m_swapChainImages.data());

  m_swapChainImageFormat = surfaceFormat.format;
  m_swapChainExtent = extent;
}

void SwapChain::CreateImageViews() {
  m_swapChainImageViews.resize(m_swapChainImages.size());

  for (size_t i = 0; i < m_swapChainImages.size(); i++) {
    m_swapChainImageViews[i] = CreateImageView(*m_device, m_swapChainImages[i],
                                               m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void SwapChain::CreateFramebuffers(VkRenderPass RenderPass) {
  m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

  for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {
      m_swapChainImageViews[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = RenderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = m_swapChainExtent.width;
    framebufferInfo.height = m_swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(*m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
      ExitWithError("Failed to create graphics pipeline!", -1);
    }
  }
}

