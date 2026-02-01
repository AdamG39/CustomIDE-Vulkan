#include "../helpers/errors/errors.hpp"
#include "renderer.hpp"
#include "vulkanCore.hpp"
#include "shapes.hpp"
#include "texture.hpp"
#include "../io/io.hpp"
#include <set>

#define APP_VERSION VK_MAKE_VERSION(0, 1, 0)

void VulkanRenderer::Start() {
  InitGLFW();
  InitVulkan();
}

// Initialise GLFW or exit with an error if failed
void VulkanRenderer::InitGLFW(){
  if (!glfwInit()) ExitWithError("GLFW Failed to Initialise", -1);
  CreateWindow();
}

void VulkanRenderer::CreateWindow() {
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  m_window = glfwCreateWindow(WIDTH, HEIGHT, m_appName.c_str(), nullptr, nullptr);

  glfwSetWindowCloseCallback(m_window, CloseWindowCallback);
  glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
  glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
  glfwSetCursorPosCallback(m_window, CursorPositionCallback);

  if (!m_window) ExitWithError("Failed to create window!", -1);
}

void VulkanRenderer::InitVulkan() {
  CreateInstance();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  InitSwapChain();
  CreateCommandPool();
  CreateCommandBuffers();
  CreateGraphicsPipeline();
  CreateSyncObjects();
}

void VulkanRenderer::CreateInstance() {
  if (m_enableValidationLayers && !CheckValidationLayerSupport(m_validationLayers.data(), m_validationLayers.size())) {
    ExitWithError("Validation layers requested, but not available!", -1);
  }

  VkApplicationInfo appInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "CustomIDE",
    .applicationVersion = APP_VERSION,
    .pEngineName = "No Engine",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_0
  };

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (m_enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
    createInfo.ppEnabledLayerNames = m_validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtenstions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtenstions;

  if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
    ExitWithError("Failed to create Vulkan instance", -1);
  }
}

void VulkanRenderer::CreateSurface() {
  if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
    ExitWithError("Failed to create window surface!", -1);
  }
}

void VulkanRenderer::PickPhysicalDevice() {
  // Get the number of physical devices installed saved to deviceCount
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

  // If no GPU's are found that support Vulkan throw error
  if (deviceCount == 0) {
      ExitWithError("Failed to find GPUs with Vulkan support!", -1);
  }

  // Load a pointer to each physical device into devices
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

  // Check if any of the GPU's found are suitable
  for (const auto& device : devices) {
    if (IsDeviceSuitable(device, m_surface, m_deviceExtensions)) {
      m_physicalDevice = device;
      break;
    }
  }

  // Throw error if no suitable GPU is found
  if (m_physicalDevice == VK_NULL_HANDLE) {
    ExitWithError("Failed to find a suitable GPU!", -1);
  }
}

void VulkanRenderer::CreateLogicalDevice() {
  QueueFamilyIndicies indicies = FindQueueFamilies(m_physicalDevice, m_surface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
    indicies.graphicsFamily.value(),
    indicies.presentFamily.value()
  };

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  
  VkPhysicalDeviceRobustness2FeaturesEXT robustnessFeatures {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
    .nullDescriptor = VK_TRUE
  };

  VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
    .pNext = &robustnessFeatures,
    .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
    .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
    .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
    .descriptorBindingPartiallyBound = VK_TRUE,
    .descriptorBindingVariableDescriptorCount = VK_TRUE,
    .runtimeDescriptorArray = VK_TRUE
  };

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = &indexingFeatures;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

  if (m_enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
    createInfo.ppEnabledLayerNames = m_validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
    ExitWithError("Failed to create logical device!", -1);
  }

  vkGetDeviceQueue(m_device, indicies.graphicsFamily.value(), 0, &m_graphicsQueue);
  vkGetDeviceQueue(m_device, indicies.presentFamily.value(), 0, &m_presentQueue);
}

void VulkanRenderer::InitSwapChain() {
  m_swapchain = new SwapChain(m_window, m_device, m_physicalDevice, m_surface);

  m_swapchain->CreateSwapChain();
  m_swapchain->CreateImageViews();

  m_renderPass = CreateRenderPass(m_swapchain->GetImageFormat(), m_device);

  m_swapchain->CreateFramebuffers(m_renderPass);
}

void VulkanRenderer::RecreateSwapChain() {
  m_swapchain->RecreateSwapChain(m_renderPass);
}

void VulkanRenderer::CreateDescriptorSets(VulkanTexture* Textures, int NumImages) {
  CreateDescriptorPool(NumImages);

  CreateDescriptorSetLayout(NumImages);

  AllocateDescriptorSets(NumImages);

  UpdateDescriptorSets(Textures, NumImages);
}

void VulkanRenderer::CreateDescriptorPool(int NumImages) {
  std::vector<VkDescriptorPoolSize> poolSizes;

  VkDescriptorPoolSize imagesPoolSize{
    .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    .descriptorCount = (uint32_t)NumImages
  };

  poolSizes.push_back(imagesPoolSize);

  VkDescriptorPoolSize samplerPoolSize{
    .type = VK_DESCRIPTOR_TYPE_SAMPLER,
    .descriptorCount = 1
  };

  poolSizes.push_back(samplerPoolSize);

  VkDescriptorPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = 0,
    .maxSets = (uint32_t)NumImages + 1, // + 1 for the sampler
    .poolSizeCount = (uint32_t)poolSizes.size(),
    .pPoolSizes = poolSizes.data()
  };

  if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    ExitWithError("Failed to create descriptor pool", 1);
}

void VulkanRenderer::CreateDescriptorSetLayout(int NumImages) {
	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

	VkDescriptorSetLayoutBinding samplerLayoutBinding {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};

  LayoutBindings.push_back(samplerLayoutBinding);

	VkDescriptorSetLayoutBinding imageLayoutBinding {
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.descriptorCount = (uint32_t)NumImages,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};

  LayoutBindings.push_back(imageLayoutBinding);

	VkDescriptorSetLayoutCreateInfo LayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext = nullptr,
		.flags = 0,
		.bindingCount = (uint32_t)LayoutBindings.size(),
		.pBindings = LayoutBindings.data()
	};

	if (vkCreateDescriptorSetLayout(m_device, &LayoutInfo, NULL, &m_descriptorSetLayout) != VK_SUCCESS)
    ExitWithError("Failed to create descriptor set", 1);
}

void VulkanRenderer::AllocateDescriptorSets(int NumImages) {
  VkDescriptorSetAllocateInfo allocateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext = nullptr,
    .descriptorPool = m_descriptorPool,
    .descriptorSetCount = 1,
    .pSetLayouts = &m_descriptorSetLayout
  };

  m_descriptorSets.resize(1);

  if (vkAllocateDescriptorSets(m_device, &allocateInfo, m_descriptorSets.data()) != VK_SUCCESS)
    ExitWithError("Failed to allocate for descriptor sets", 1);
}

void VulkanRenderer::UpdateDescriptorSets(VulkanTexture* Textures, int NumImages) {
  VkDescriptorImageInfo imageInfos[MAX_TEXTURES];

  VkFilter minFilter = VK_FILTER_LINEAR;
  VkFilter maxFilter = VK_FILTER_LINEAR;
  VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  m_sampler = CreateTextureSampler(m_device, minFilter, maxFilter, addressMode);

  VkDescriptorImageInfo samplerInfo {};
    
  samplerInfo.sampler = m_sampler;

  for (uint32_t i = 0; i < MAX_TEXTURES; i++) {
    imageInfos[i].sampler = nullptr;
    imageInfos[i].imageView = Textures[i].view;
    imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }

  VkWriteDescriptorSet setWrites[2];

  setWrites[0] = {};
  setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  setWrites[0].dstSet = m_descriptorSets[0];
  setWrites[0].dstBinding = 0;
  setWrites[0].dstArrayElement = 0;
  setWrites[0].descriptorCount = 1;
  setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  setWrites[0].pImageInfo = &samplerInfo;
  setWrites[0].pBufferInfo = nullptr;

  setWrites[1] = {};
  setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  setWrites[1].dstSet = m_descriptorSets[0];
  setWrites[1].dstBinding = 1;
  setWrites[1].dstArrayElement = 0;
  setWrites[1].descriptorCount = (uint32_t)MAX_TEXTURES;
  setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  setWrites[1].pImageInfo = imageInfos;
  setWrites[1].pBufferInfo = nullptr;

  vkUpdateDescriptorSets(m_device, 2, setWrites, 0, nullptr);
}

void VulkanRenderer::LoadImages(const std::vector<std::string>& filePaths) {
  int i = 0;
  for (std::string filepath : filePaths) {
    m_textures[i] = VulkanTexture();
  }
}

void VulkanRenderer::CreateGraphicsPipeline() {
  std::vector<std::string> filePaths = {
    "../assets/textures/pngTest.png",
    "../assets/textures/test.bmp",
    "../assets/textures/cross.png",
    "../assets/unscii-alt-font.png",
    "../assets/unscii-alt-font-32.png"
  };
  LoadImages(filePaths);

  CreateDescriptorSets(m_textures, MAX_TEXTURES/*MAX_FRAMES_IN_FLIGHT*/);

  auto vertShaderCode = ReadBinaryFile("../shaders/vert.spv");
  auto fragShaderCode = ReadBinaryFile("../shaders/frag.spv");

  VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode.data(), vertShaderCode.size(), m_device);
  VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode.data(), fragShaderCode.size(), m_device);

  VkPipelineShaderStageCreateInfo shaderStages[2] = {
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertShaderModule,
      .pName = "main"
    },
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragShaderModule,
      .pName = "main"
    }
  };

  using VertexF = Vertex<float, float>;

  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(VertexF);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(VertexF, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(VertexF, colour);

  attributeDescriptions[2].binding = 0;
  attributeDescriptions[2].location = 2;
  attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[2].offset = offsetof(VertexF, textureCoords);

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colourBlendAttachment{
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
  };

  VkPipelineColorBlendStateCreateInfo colourBlending{};
  colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colourBlending.logicOpEnable = VK_FALSE;
  colourBlending.logicOp = VK_LOGIC_OP_COPY;
  colourBlending.attachmentCount = 1;
  colourBlending.pAttachments = &colourBlendAttachment;
  colourBlending.blendConstants[0] = 0.0f;
  colourBlending.blendConstants[1] = 0.0f;
  colourBlending.blendConstants[2] = 0.0f;
  colourBlending.blendConstants[3] = 0.0f;

  std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPushConstantRange pushConstantRanges[2];

  pushConstantRanges[0] = {};
  pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRanges[0].offset = 0;
  pushConstantRanges[0].size = sizeof(PushConstants);

  pushConstantRanges[1] = {};
  pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRanges[1].offset = sizeof(PushConstants);
  pushConstantRanges[1].size = sizeof(int);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
  pipelineLayoutInfo.pushConstantRangeCount = 2;
  pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;

  if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
    ExitWithError("Failed to create pipeline layout!", -1);
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr;
  pipelineInfo.pColorBlendState = &colourBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.renderPass = m_renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
    ExitWithError("Failed to create graphics pipeline!", -1);
  }

  vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
  vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
}

void VulkanRenderer::CreateCommandPool() {
  QueueFamilyIndicies queueFamilyIndicies = FindQueueFamilies(m_physicalDevice, m_surface);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndicies.graphicsFamily.value();

  if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
    ExitWithError("Failed to create command pool!", -1);
  }
}

void VulkanRenderer::CreateCommandBuffers() {
  m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = m_commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = (uint32_t) m_commandBuffers.size()
  };

  if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
    ExitWithError("Failed to allocate command buffers!", -1);
  }
}

uint32_t VulkanRenderer::FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((TypeFilter & (1 << i)) &&
      (memProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
      return i;
    }
  }

  ExitWithError("Failed to find suitable memory type", -1);
  return -1; // This point will never get reached
}

void VulkanRenderer::CreateVertexBuffer() {
  if (m_vertexArray.empty()) {
    ExitWithError("Vertex array is empty!", -1);
  }

  // Increase the total size by double to reduce calls
  size_t requiredVertexCount = m_vertexArray.size() * 2;

  if (m_vertexBuffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
  }

  VkDeviceSize bufferSize = requiredVertexCount * sizeof(Vertex<float, float>);

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS) {
    ExitWithError("Failed to create vertex buffer", -1);
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(
    memRequirements.memoryTypeBits,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  );

  if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS) {
    ExitWithError("Failed to allocated vertex buffer memory", -1);
  }

  vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexBufferMemory, 0);

  m_vertexBufferCapacity = requiredVertexCount;
}

void VulkanRenderer::UploadVertexData() {
  VkDeviceSize bufferSize = m_vertexArray.size() * sizeof(Vertex<int, float>);

  void* data;
  vkMapMemory(m_device, m_vertexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, m_vertexArray.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(m_device, m_vertexBufferMemory);
}

void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                                         const std::vector<TextureArrayBounds>& bounds) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    ExitWithError("Failed to begin recording command buffer!", -1);
  }

  VkExtent2D extent = m_swapchain->GetExtent();

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_renderPass;
  renderPassInfo.framebuffer = m_swapchain->GetFramebuffers()[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = extent;
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &m_clearColour;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(extent.width),
      .height = static_cast<float>(extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{
      .offset = {0, 0},
      .extent = extent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

    PushConstants pc{};
    pc.width = static_cast<float>(framebufferWidth);
    pc.height = static_cast<float>(framebufferHeight);

    vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
        0, sizeof(PushConstants), &pc);


    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout,
                            0, 1, &m_descriptorSets[0], 0, nullptr);

    for (TextureArrayBounds bound : bounds) {
      int textureIndex = bound.textureIndex;
      vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT,
          sizeof(PushConstants), sizeof(int), (void *)&textureIndex);

      vkCmdDraw(commandBuffer, bound.bounds.count, 1, bound.bounds.start, 0);
    }

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    ExitWithError("Failed to record command buffer!", -1);
  }
}

void VulkanRenderer::CreateSyncObjects() {
  m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
      ExitWithError("Failed to create semaphores!", -1);
    }
  }
}

void VulkanRenderer::DrawFrame() {
  vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
  vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

  uint32_t imageIndex = 0;
  VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain->GetSwapChain(), UINT64_MAX, 
                                          m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
    return;
  }

  vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

  // If buffer size is not sufficient recreate it
  if (m_vertexArray.size() > m_vertexBufferCapacity) {
    CreateVertexBuffer();
  }

  if (m_vertexArray.size() != 0) {
    UploadVertexData();

    RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex, m_textureIndexArrayBounds);
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

  VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
    ExitWithError("Failed to submit draw command buffer!", -1);
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {m_swapchain->GetSwapChain()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    RecreateSwapChain();
  }

  m_vertexArray.clear();

  m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::FillVertexBuffer(std::vector<Vertex<float, float>> Vertices,
                                      std::vector<TextureArrayBounds> textureIndexArrayBounds) {
  m_vertexArray = std::move(Vertices);
  m_textureIndexArrayBounds = std::move(textureIndexArrayBounds);
}

void VulkanRenderer::Cleanup() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
    vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
    vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(m_device, m_commandPool, nullptr);
  delete m_swapchain;
  vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
  for (auto tex : m_textures) {
    tex.Destroy(m_device);
  }
  vkDestroySampler(m_device, m_sampler, nullptr);
  vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
  vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
  vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
  vkDestroyDevice(m_device, nullptr);
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  vkDestroyInstance(m_instance, nullptr);
  glfwTerminate();
}

