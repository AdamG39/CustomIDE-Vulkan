#ifndef CUSTOM_VULKAN_RENDERER_H
#define CUSTOM_VULKAN_RENDERER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "vulkanCore.hpp"
#include "shapes.hpp"
#include "texture.hpp"
#include "buffer.hpp"
#include <string>
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRenderer {
public:
  VulkanRenderer(std::string AppName, Colour<float> ClearColour)
  : m_appName(AppName)
  {
    Start();
    SetClearColour(ClearColour);
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

  VkSwapchainKHR m_swapChain;
  std::vector<VkImage> m_swapChainImages;
  VkFormat m_swapChainImageFormat;
  VkExtent2D m_swapChainExtent;
  std::vector<VkImageView> m_swapChainImageViews;
  std::vector<VkFramebuffer> m_swapChainFramebuffers;

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

  void CreateSwapChain();

  void CreateImageViews();

  void CreateRenderPass();

  VkShaderModule CreateShaderModule(const std::vector<char>& code);

  void CreateDescriptorSets(VulkanTexture* pTexture, int NumImages);

  void CreateDescriptorPool(int NumImages);

  void CreateDescriptorSetLayout(VulkanTexture* pTexture);

  void AllocateDescriptorSets(int NumImages);

  void UpdateDescriptorSets(VulkanTexture* pTexture, int NumImages);

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

  VulkanBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);

  void CreateTexture(const char* pFilename, VulkanTexture& Texture);

  void CreateTextureImageFromData(VulkanTexture& Texture, const void* pPixels,
                                  uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TextureFormat);

  void CreateTextureImage(VulkanTexture& Texture, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TextureFormat,
                          VkImageUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags);

  void UpdateTextureImage(VulkanTexture& Texture, uint32_t ImageWidth, uint32_t ImageHeight,
                          VkFormat TextureFormat, const void* pPixels);

  int GetBytesPerTextureFormat(VkFormat TextureFormat);

  uint32_t GetMemoryTypeIndex(uint32_t MemoryTypeBitsMask, VkMemoryPropertyFlags RequiredMemoryPropertyFlags);

  void TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout);

  void ImageMemoryBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format,
                       VkImageLayout OldLayout, VkImageLayout NewLayout);

  void CopyBufferToImage(VkImage Destination, VkBuffer Source, uint32_t ImageWidth, uint32_t ImageHeight);

  void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags);

  void SubmitCopyCommand();
};

#endif

