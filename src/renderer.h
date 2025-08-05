#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <optional>
#include "shapes.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

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

class VulkanRenderer {
public:
  VulkanRenderer(std::string AppName)
  : m_appName(AppName)
  {
    Start();
  }

  ~VulkanRenderer() {
    Cleanup();
  }

  void DrawFrame();

  void FillVertexBuffer(std::vector<Vertex<float, float>> Vertices);

  GLFWwindow* GetWindow() const { return m_window; }

  VkDevice GetDevice() const { return m_device; }

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

  VkSwapchainKHR m_swapChain;
  std::vector<VkImage> m_swapChainImages;
  VkFormat m_swapChainImageFormat;
  VkExtent2D m_swapChainExtent;
  std::vector<VkImageView> m_swapChainImageViews;
  std::vector<VkFramebuffer> m_swapChainFramebuffers;

  VkRenderPass m_renderPass;
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

  bool CheckValidationLayerSupport();

  bool IsDeviceSuitable(VkPhysicalDevice device);

  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

  QueueFamilyIndicies FindQueueFamilies(VkPhysicalDevice device);

  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

  VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

  void PickPhysicalDevice();

  void CreateLogicalDevice();

  void CreateSwapChain();

  void RecreateSwapChain();

  void CreateImageViews();

  void CreateRenderPass();

  VkShaderModule CreateShaderModule(const std::vector<char>& code);

  void CreateGraphicsPipeline();

  void CreateFramebuffers();

  void CreateCommandPool();

  void CreateCommandBuffers();

  uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

  void CreateVertexBuffer();

  void UploadVertexData();

  void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

  void CreateSyncObjects();

  void Cleanup();
};

void CloseWindowCallback(GLFWwindow* window);

void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods);
