#ifndef CUSTOM_VULKAN_RENDERER_H
#define CUSTOM_VULKAN_RENDERER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <list>
#include "swapChain.hpp"
#include "shapes.hpp"
#include "texture.hpp"

namespace Vulkan {

class Renderer {
public:
  Renderer(std::string AppName, CustomIDE::Colour ClearColour)
  : m_appName(AppName)
  {
    SetClearColour(ClearColour);
    Start();
  }

  ~Renderer() {
    Cleanup();
  }

  void DrawRect(CustomIDE::Rect2D Rect, int ZIndex, CustomIDE::Colour Colour = {0x000000, 1.f});
  void DrawRectEx(CustomIDE::Rect2D Rect, int ZIndex, CustomIDE::ClipRect ClipArea,
                  CustomIDE::Colour Colour = {0x000000, 1.f});

  void DrawTexturedRect(CustomIDE::Rect2D Rect, CustomIDE::UVRect2D UVRect, int ZIndex, CustomIDE::TextureID TextureIndex);
  void DrawTexturedRectEx(CustomIDE::Rect2D Rect, CustomIDE::UVRect2D UVRect, int ZIndex, CustomIDE::TextureID TextureIndex,
                          CustomIDE::ClipRect ClipArea, CustomIDE::Colour Colour = {0x000000, 1.f});

  void DrawFrame();

  GLFWwindow* GetWindow() const { return m_window; }

  VkDevice GetDevice() const { return m_device; }

  void RecreateSwapChain();

  void SetClearColour(const CustomIDE::Colour ClearColour) {
    m_clearColour = {{{ClearColour.r, ClearColour.g, ClearColour.b, ClearColour.a}}};
  }

  void LoadImage(const std::string& Filepath, bool UpdateDescriptors = false);

  long long GetImageIndexFromName(const std::string& FileName) const {
    auto pos = m_textures.Contains(FileName);
    return (pos != m_textures.Size()) ? pos : -1;
  }

  CustomIDE::Vector2<int> GetImageDimensions(size_t ImageIndex) const {
    return m_textures[ImageIndex].metadata.dimensions;
  }

  CustomIDE::Vector2D GetWindowContentScale();

  void SetWindowContentScale(float xScale, float yScale);

private:
  static const uint32_t WIDTH = 1920;
  static const uint32_t HEIGHT = 1080;
  static const uint32_t MAX_TEXTURES = 100;

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

  uint32_t m_maxFramesInFlight = 2;

  CustomIDE::Vector2D m_windowContentScale = { 1.f, 1.f };

  VkInstance m_instance;
  VkSurfaceKHR m_surface;

  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device;

  VkQueue m_graphicsQueue;
  VkQueue m_presentQueue;

  SwapChain* m_swapchain = nullptr;

  TextureBufferMap m_textures {(size_t)MAX_TEXTURES};
  VkSampler m_sampler = VK_NULL_HANDLE;

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
  std::vector<CustomIDE::Vertex<float>> m_vertexArray;
  std::list<DrawCommand> m_commands;
  std::vector<DrawBatch> m_drawBatches;
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

  void LoadImages(const std::vector<std::string>& Filepaths);

  void CreateDescriptorSets(const TextureBufferMap& Textures, int NumImages);

  void CreateDescriptorPool(int NumImages);

  void CreateDescriptorSetLayout(int NumImages);

  void AllocateDescriptorSets(int NumImages);

  void UpdateDescriptorSets(const TextureBufferMap& Textures, int NumImages);

  void CreateGraphicsPipeline();

  void CreateCommandPool();

  void CreateCommandBuffers();

  uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

  void CreateVertexBuffer();

  void UploadVertexData();

  void StartRenderPass(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

  void RecordCommandBuffer(VkCommandBuffer commandBuffer,
                           const std::vector<DrawBatch>& batches);

  void EndRenderPass(VkCommandBuffer& commandBuffer);

  void CreateSyncObjects();

  void BatchDrawCommands();

  void FillVertexArray();

  void Cleanup();
};

} // namespace Vulkan

#endif

