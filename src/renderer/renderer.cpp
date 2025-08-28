#include "../helpers/errors/errors.hpp"
#include "renderer.hpp"
#include "shapes.hpp"
#include "../io/io.hpp"
#include <set>

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
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFramebuffers();
  CreateCommandPool();
  CreateCommandBuffers();
  CreateSyncObjects();
}

void VulkanRenderer::CreateInstance() {
  if (m_enableValidationLayers && !CheckValidationLayerSupport(m_validationLayers.data(), m_validationLayers.size())) {
    ExitWithError("Validation layers requested, but not available!", -1);
  }

  VkApplicationInfo appInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "CustomIDE",
    .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
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
  const char** glfwExtenstions;

  glfwExtenstions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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
  std::set<uint32_t> uniqueQueueFamilies = {indicies.graphicsFamily.value(), indicies.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
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

void VulkanRenderer::CreateSwapChain() {
  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount;

  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndicies indicies = FindQueueFamilies(m_physicalDevice);
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

  if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
    ExitWithError("Failed to create swap chain!", -1);
  }

  vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
  m_swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

  m_swapChainImageFormat = surfaceFormat.format;
  m_swapChainExtent = extent;
}

void VulkanRenderer::RecreateSwapChain() {
  if (glfwWindowShouldClose(m_window)) { return; }
  vkDeviceWaitIdle(m_device);

  for (auto framebuffer: m_swapChainFramebuffers) {
    vkDestroyFramebuffer(m_device, framebuffer, nullptr);
  }
  m_swapChainFramebuffers.clear();

  for (auto imageView: m_swapChainImageViews) {
    vkDestroyImageView(m_device, imageView, nullptr);
  }
  m_swapChainImageViews.clear();

  vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

  CreateSwapChain();
  CreateImageViews();
  CreateFramebuffers();
}

void VulkanRenderer::CreateImageViews() {
  m_swapChainImageViews.resize(m_swapChainImages.size());

  for (size_t i = 0; i < m_swapChainImages.size(); i++) {
    m_swapChainImageViews[i] = CreateImageView(m_device, m_swapChainImages[i],
                                               m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    ExitWithError("Failed to create shader module", -1);
  }

  return shaderModule;
}

void VulkanRenderer::CreateRenderPass() {
  VkAttachmentDescription colourAttachment{};
  colourAttachment.format = m_swapChainImageFormat;
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

  if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
    ExitWithError("Failed to create render pass!", -1);
  }
}

void VulkanRenderer::CreateDescriptorSets(VulkanTexture* pTexture, int NumImages) {
  CreateDescriptorPool(NumImages);

  CreateDescriptorSetLayout(pTexture);

  AllocateDescriptorSets(NumImages);

  UpdateDescriptorSets(pTexture, NumImages);
}

void VulkanRenderer::CreateDescriptorPool(int NumImages) {
  VkDescriptorPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = 0,
    .maxSets = (uint32_t)NumImages,
    .poolSizeCount = 0,
    .pPoolSizes = nullptr
  };

  if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    ExitWithError("Failed to create descriptor pool", 1);
}

void VulkanRenderer::CreateDescriptorSetLayout(VulkanTexture* pTex) {
	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

	VkDescriptorSetLayoutBinding fragmentShaderLayoutBinding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};

	if (pTex) { 
		LayoutBindings.push_back(fragmentShaderLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo LayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,			// reserved - must be zero
		.bindingCount = (uint32_t)LayoutBindings.size(),
		.pBindings = LayoutBindings.data()
	};

	if (vkCreateDescriptorSetLayout(m_device, &LayoutInfo, NULL, &m_descriptorSetLayout) != VK_SUCCESS)
    ExitWithError("Failed to create descriptor set", 1);
}

void VulkanRenderer::AllocateDescriptorSets(int NumImages) {
  std::vector<VkDescriptorSetLayout> layouts(NumImages, m_descriptorSetLayout);

  VkDescriptorSetAllocateInfo allocateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext = nullptr,
    .descriptorPool = m_descriptorPool,
    .descriptorSetCount = (uint32_t)NumImages,
    .pSetLayouts = layouts.data()
  };

  m_descriptorSets.resize(NumImages);

  if (vkAllocateDescriptorSets(m_device, &allocateInfo, m_descriptorSets.data()) != VK_SUCCESS)
    ExitWithError("Failed to allocate for descriptor sets", 1);
}

void VulkanRenderer::UpdateDescriptorSets(VulkanTexture* pTexture, int NumImages) {
  VkDescriptorImageInfo imageInfo;

  if (pTexture) {
    imageInfo.sampler = pTexture->sampler;
    imageInfo.imageView = pTexture->view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }

  std::vector<VkWriteDescriptorSet> writeDescriptorSets;

  for (size_t i = 0; i < NumImages; i++) {
    if (pTexture) {
      writeDescriptorSets.push_back(
        VkWriteDescriptorSet{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = m_descriptorSets[i],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &imageInfo
        }
      );
    }
  }

  vkUpdateDescriptorSets(m_device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
}

void VulkanRenderer::CreateGraphicsPipeline() {
  CreateTexture("../assets/textures/test.bmp", m_texture);

  CreateDescriptorSets(&m_texture, MAX_FRAMES_IN_FLIGHT);

  auto vertShaderCode = ReadBinaryFile("../shaders/vert.spv");
  auto fragShaderCode = ReadBinaryFile("../shaders/frag.spv");

  VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
  VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

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

  std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(VertexF, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(VertexF, colour);

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

  VkPipelineColorBlendAttachmentState colourBlendAttachment{};
  colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colourBlendAttachment.blendEnable = VK_FALSE;

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

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstants);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0; // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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

void VulkanRenderer::CreateFramebuffers() {
  m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

  for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {
      m_swapChainImageViews[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = m_swapChainExtent.width;
    framebufferInfo.height = m_swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
      ExitWithError("Failed to create graphics pipeline!", -1);
    }
  }
}

void VulkanRenderer::CreateCommandPool() {
  QueueFamilyIndicies queueFamilyIndicies = FindQueueFamilies(m_physicalDevice);

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

void VulkanRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    ExitWithError("Failed to begin recording command buffer!", -1);
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_renderPass;
  renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_swapChainExtent;
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &m_clearColour;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(m_swapChainExtent.width);
  viewport.height = static_cast<float>(m_swapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = m_swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  VkBuffer vertexBuffers[] = { m_vertexBuffer };
  VkDeviceSize offsets[] = { 0 };

  int framebufferWidth, framebufferHeight;
  glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

  PushConstants pc{};
  pc.width = static_cast<float>(framebufferWidth);
  pc.height = static_cast<float>(framebufferHeight);

  vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pc);

  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_vertexArray.size()), 1, 0, 0);

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
  VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, 
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

    RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);
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

  VkSwapchainKHR swapChains[] = {m_swapChain};
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

void VulkanRenderer::FillVertexBuffer(std::vector<Vertex<float, float>> Vertices) {
  // TODO:
  // Should actually take an array of triangles, sort via Z index 
  // then split into a single array vertices

  m_vertexArray = Vertices;
}

void VulkanRenderer::Cleanup() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
    vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
    vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(m_device, m_commandPool, nullptr);
  for (auto framebuffer: m_swapChainFramebuffers) {
    vkDestroyFramebuffer(m_device, framebuffer, nullptr);
  }
  vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  for (auto imageView: m_swapChainImageViews) {
    vkDestroyImageView(m_device, imageView, nullptr);
  }
  vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
  vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
  vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
  vkDestroyDevice(m_device, nullptr);
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  vkDestroyInstance(m_instance, nullptr);
  glfwTerminate();
}

VulkanBuffer VulkanRenderer::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties) {
  VkBufferCreateInfo bufferInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = Size,
    .usage = Usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VulkanBuffer buffer;

  if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
    ExitWithError("Failed to create buffer", -18);

  VkMemoryRequirements memoryRequirements = { 0 };
  vkGetBufferMemoryRequirements(m_device, buffer.buffer, &memoryRequirements);

  buffer.allocationSize = memoryRequirements.size;

  uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, Properties);

  VkMemoryAllocateInfo memoryAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = nullptr,
    .allocationSize = memoryRequirements.size,
    .memoryTypeIndex = memoryTypeIndex
  };

  if (vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &buffer.memory) != VK_SUCCESS)
    ExitWithError("Failed to allocate memory for image", -14);

  if (vkBindBufferMemory(m_device, buffer.buffer, buffer.memory, 0) != VK_SUCCESS)
    ExitWithError("Failed to bind image memory", -15);

  return buffer;
}

void VulkanRenderer::CreateTexture(const char* pFilename, VulkanTexture& Texture) {
  std::vector<std::shared_ptr<Image>> image;
  if (!ReadImageFile(pFilename, image)) ExitWithError("Failed to load file", -11);

  VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
  CreateTextureImageFromData(Texture, image[0]->pixels, image[0]->width, image[0]->height, format);

  Texture.view = CreateImageView(m_device, Texture.image, format, VK_IMAGE_ASPECT_COLOR_BIT);

  VkFilter minFilter = VK_FILTER_LINEAR;
  VkFilter maxFilter = VK_FILTER_LINEAR;
  VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  Texture.sampler = CreateTextureSampler(m_device, minFilter, maxFilter, addressMode);
}

void VulkanRenderer::CreateTextureImageFromData(VulkanTexture& Texture, const void* pPixels,
                                                uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TextureFormat) {
  VkImageUsageFlagBits usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                      VK_IMAGE_USAGE_SAMPLED_BIT);
  VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  CreateTextureImage(Texture, ImageWidth, ImageHeight, TextureFormat, usage, propertyFlags);

  UpdateTextureImage(Texture, ImageWidth, ImageHeight, TextureFormat, pPixels);
}

void VulkanRenderer::CreateTextureImage(VulkanTexture& Texture, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TextureFormat,
                                        VkImageUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags) {
  VkImageCreateInfo imageInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = TextureFormat,
    .extent = VkExtent3D { .width = ImageWidth, .height = ImageHeight, .depth = 1 },
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = UsageFlags,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = nullptr,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
  };

  if (vkCreateImage(m_device, &imageInfo, NULL, &Texture.image) != VK_SUCCESS)
      ExitWithError("Failed to create image", -12);

  VkMemoryRequirements memoryRequirements = { 0 };
  vkGetImageMemoryRequirements(m_device, Texture.image, &memoryRequirements);

  uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, PropertyFlags);

  VkMemoryAllocateInfo memoryAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = nullptr,
    .allocationSize = memoryRequirements.size,
    .memoryTypeIndex = memoryTypeIndex
  };

  if (vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &Texture.memory) != VK_SUCCESS)
    ExitWithError("Failed to allocate memory for image", -14);

  if (vkBindImageMemory(m_device, Texture.image, Texture.memory, 0) != VK_SUCCESS)
    ExitWithError("Failed to bind image memory", -15);
}

void VulkanRenderer::UpdateTextureImage(VulkanTexture& Texture, uint32_t ImageWidth, uint32_t ImageHeight,
                        VkFormat TextureFormat, const void* pPixels) {
  int bytesPerPixel = GetBytesPerTextureFormat(TextureFormat);

  VkDeviceSize layerSize = ImageWidth * ImageHeight * bytesPerPixel;
  int layerCount = 1;
  VkDeviceSize imageSize = layerCount * layerSize;

  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  VulkanBuffer stagingBuffer = CreateBuffer(imageSize, usage, properties);

  stagingBuffer.Update(m_device, pPixels, imageSize);

  TransitionImageLayout(Texture.image, TextureFormat,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  CopyBufferToImage(Texture.image, stagingBuffer.buffer, ImageWidth, ImageHeight);

  TransitionImageLayout(Texture.image, TextureFormat,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  stagingBuffer.Destroy(m_device);
}

int VulkanRenderer::GetBytesPerTextureFormat(VkFormat TextureFormat) {
  switch (TextureFormat) {
  case VK_FORMAT_R8_SINT:
  case VK_FORMAT_R8_UNORM:
    return 1;
  case VK_FORMAT_R16_SFLOAT:
    return 2;
  case VK_FORMAT_R16G16_SFLOAT:
  case VK_FORMAT_R16G16_SNORM:
  case VK_FORMAT_B8G8R8A8_UNORM:
  case VK_FORMAT_R8G8B8A8_UNORM:
    return 4;
  case VK_FORMAT_R16G16B16A16_SFLOAT:
    return 4 * sizeof(uint16_t);
  case VK_FORMAT_R32G32B32A32_SFLOAT:
    return 4 * sizeof(float);
  default:
    ExitWithError("Not support format", -16);
  }

  return 0;
}

uint32_t VulkanRenderer::GetMemoryTypeIndex(uint32_t MemoryTypeBitsMask, VkMemoryPropertyFlags RequiredMemoryPropertyFlags) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    VkMemoryType memoryType = memoryProperties.memoryTypes[i];
    uint32_t currentBitmask = (1 << i);
    bool isCurrentMemoryTypeSupported = (MemoryTypeBitsMask & currentBitmask);
    bool hasRequiredMemoryProperties = ((memoryType.propertyFlags & RequiredMemoryPropertyFlags) == RequiredMemoryPropertyFlags);

    if (isCurrentMemoryTypeSupported && hasRequiredMemoryProperties) return i;
  }

  ExitWithError("No valid memory type found", -13);
  return -1;
}

void VulkanRenderer::TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout) {
  BeginCommandBuffer(m_commandBuffers[m_currentFrame], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  ImageMemoryBarrier(m_commandBuffers[m_currentFrame], Image, Format, OldLayout, NewLayout);

  SubmitCopyCommand();
}

void VulkanRenderer::ImageMemoryBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format,
                                     VkImageLayout OldLayout, VkImageLayout NewLayout) {
  VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = OldLayout,
		.newLayout = NewLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = Image,
		.subresourceRange = VkImageSubresourceRange {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

	if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
		(Format == VK_FORMAT_D16_UNORM) ||
		(Format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
		(Format == VK_FORMAT_D32_SFLOAT) ||
		(Format == VK_FORMAT_S8_UINT) ||
		(Format == VK_FORMAT_D16_UNORM_S8_UINT) ||
		(Format == VK_FORMAT_D24_UNORM_S8_UINT))
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (Format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        Format == VK_FORMAT_D24_UNORM_S8_UINT) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_GENERAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
		NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} /* Convert back from read-only to updateable */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} /* Convert from updateable texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
		     NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} /* Convert depth texture from undefined state to depth-stencil buffer */
	else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} /* Wait for render pass to complete */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = 0;
		/*
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		///		destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		*/
		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} /* Convert back from read-only to color attachment */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} /* Convert from updateable texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} /* Convert back from read-only to depth attachment */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	} /* Convert from updateable depth texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} 
	else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	else {
		ExitWithError("Unknown barrier case", 1);
	}

	vkCmdPipelineBarrier(CmdBuf, sourceStage, destinationStage, 
		                 0, 0, NULL, 0, NULL, 1, &barrier);
}

void VulkanRenderer::CopyBufferToImage(VkImage Destination, VkBuffer Source, uint32_t ImageWidth, uint32_t ImageHeight) {
  BeginCommandBuffer(m_commandBuffers[m_currentFrame], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferImageCopy BufferImageCopy = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = VkImageSubresourceLayers {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.imageOffset = VkOffset3D {.x = 0, .y = 0, .z = 0 },
		.imageExtent = VkExtent3D {.width = ImageWidth, .height = ImageHeight, .depth = 1 }
	};

	vkCmdCopyBufferToImage(m_commandBuffers[m_currentFrame], Source, Destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
						   1, &BufferImageCopy);

	SubmitCopyCommand();
}

void VulkanRenderer::BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags)
{
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = UsageFlags,
		.pInheritanceInfo = NULL
	};

  if (vkBeginCommandBuffer(CommandBuffer, &beginInfo) != VK_SUCCESS)
    ExitWithError("Failed to start command buffer", -19);
}

void VulkanRenderer::SubmitCopyCommand() {
  vkEndCommandBuffer(m_commandBuffers[m_currentFrame]);

  VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = VK_NULL_HANDLE,
		.pWaitDstStageMask = VK_NULL_HANDLE,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_commandBuffers[m_currentFrame],
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = VK_NULL_HANDLE
	};

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, NULL) != VK_SUCCESS)
    ExitWithError("Failed to submit queue", 1);

  vkQueueWaitIdle(m_graphicsQueue);
}

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format,
                     VkImageAspectFlags AspectFlags) {
  VkImageViewCreateInfo viewInfo =
	{
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

VkSampler CreateTextureSampler(VkDevice Device, VkFilter MinFilter, VkFilter MaxFilter, 
                               VkSamplerAddressMode AddressMode) {
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

