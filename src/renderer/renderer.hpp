#ifndef CUSTOM_VULKAN_RENDERER_H
#define CUSTOM_VULKAN_RENDERER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "swapChain.hpp"
#include "shapes.hpp"
#include "texture.hpp"
#include <string>
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRenderer {
public:
  VulkanRenderer(std::string AppName, Colour<float> ClearColour)
  : m_appName(AppName)
  {
    SetClearColour(ClearColour);
    Start();
  }

  ~VulkanRenderer() {
    Cleanup();
  }

  void DrawFrame();

  void FillVertexBuffer(std::vector<Vertex<float, float>> Vertices);

  GLFWwindow* GetWindow() const { return m_window; }

  VkDevice GetDevice() const { return m_device; }

  void RecreateSwapChain();

  void SetClearColour(const Colour<float> ClearColour) {
    m_clearColour = {{{ClearColour.r, ClearColour.g, ClearColour.b, ClearColour.a}}};
  }

private:
  const uint32_t WIDTH = 1920;
  const uint32_t HEIGHT = 1080;

  const std::vector<const char*> m_validationLayers = {
    "VK_LAYER_KHRONOS_validation"
  };

  const std::vector<const char*> m_deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  #ifdef NDEBUG
    const bool m_enableValidationLayers = false;
  #else
    const bool m_enableValidationLayers = true;
  #endif

  std::string m_appName;

  GLFWwindow* m_window;

  VkInstance m_instance;
  VkSurfaceKHR m_surface;

  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device;

  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;

  SwapChain* m_swapchain = nullptr;

  VulkanTexture m_texture;

  VkDescriptorPool m_descriptorPool;
  VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> m_descriptorSets;
  VkRenderPass m_renderPass;
  VkClearValue m_clearColour;
  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_graphicsPipeline;

  VkCommandPool m_commandPool;
  std::vector<VkCommandBuffer> m_commandBuffers;

  size_t m_vertexBufferCapacity = 0;
  std::vector<Vertex<float, float>> m_vertexArray;
  VkDeviceMemory m_vertexBufferMemory;
  VkBuffer m_vertexBuffer = VK_NULL_HANDLE;

  std::vector<VkSemaphore> m_imageAvailableSemaphores;
  std::vector<VkSemaphore> m_renderFinishedSemaphores;
  std::vector<VkFence> m_inFlightFences;

  uint32_t m_currentFrame = 0;

  void Start();

  void InitGLFW();

  void CreateWindow();

  void InitVulkan();

  void CreateInstance();

  void CreateSurface();

  void PickPhysicalDevice();

  void CreateLogicalDevice();

  void InitSwapChain();

  void CreateDescriptorSets(VulkanTexture* pTexture, int NumImages);

  void CreateDescriptorPool(int NumImages);

  void CreateDescriptorSetLayout(VulkanTexture* pTexture);

  void AllocateDescriptorSets(int NumImages);

  void UpdateDescriptorSets(VulkanTexture* pTexture, int NumImages);

  void CreateGraphicsPipeline();

  void CreateCommandPool();

  void CreateCommandBuffers();

  uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

  void CreateVertexBuffer();

  void UploadVertexData();

  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

  void CreateSyncObjects();

  void Cleanup();
};

#endif

