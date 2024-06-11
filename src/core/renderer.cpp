#include "renderer.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "core.h"
#include "dynamic-array.h"
#include "logger.h"
#include "render-util.h"
#include "window.h"

#include <chrono>
#include <set>
#include <vulkan/vulkan_structs.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include "primitives/vertex.h"

namespace jet
{

const vk::PhysicalDevice &Renderer::GetPhysicalDevice() const
{
  return mPhysicalDevice;
}

const vk::Device &Renderer::GetDevice() const
{
  return mDevice;
}
const vk::SurfaceKHR &Renderer::GetSurfaceKHR() const
{
  return mSurface;
}

const Window &Renderer::GetWindow() const
{
  return *mWindow;
}

void Renderer::Init(Window *window)
{
  CHECK_IN();

  mWindow = window;

  vCreateInstance();
  vSetupDebugMessenger();
  vCreateSurface();
  ru::vPickPhysicalDevice(mInstance, mPhysicalDevice);
  vCreateLogicalDevice();
  vCreateSwapchain();
  vCreateImageViews();
  vCreateRenderPass();
  vCreateDescriptorSetLayout();
  vCreateGraphicsPipeline();
  vCreateFrameBuffers();
  vCreateCommandPool();
  vCreateVertexBuffer();
  vCreateIndexBuffer();
  vCreateUniformBuffers();
  vCreateDescriptorPool();
  vCreateDescriptorSets();
  vCreateCommandBuffers();
  vCreateSyncObjects();
}

void Renderer::vCreateInstance()
{
  if (mEnableValidationLayers && !vCheckValidationLayerSupport())
  {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

  vk::ApplicationInfo appInfo{};
  appInfo.sType = vk::StructureType::eApplicationInfo;
  appInfo.pApplicationName = "Jet";
  appInfo.applicationVersion = vk::makeApiVersion(0, 0, 0, 1);
  appInfo.pEngineName = "Jet";
  appInfo.engineVersion = vk::makeApiVersion(0, 0, 0, 1);
  appInfo.apiVersion = vk::ApiVersion13;

  vk::InstanceCreateInfo createInfo{};
  createInfo.sType = vk::StructureType::eInstanceCreateInfo;
  createInfo.pApplicationInfo = &appInfo;

  createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

  std::vector<const char *> requiredExtensions = vGetRequiredExtensions();

  createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
  createInfo.ppEnabledExtensionNames = requiredExtensions.data();

  createInfo.enabledLayerCount = 0;

  if (mEnableValidationLayers)
  {
    createInfo.enabledLayerCount = (u32)mValidationLayers.size();
    createInfo.ppEnabledLayerNames = mValidationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  VK_TRY(vk::createInstance(&createInfo, nullptr, &mInstance), "Failed to create VkInstance");
}

std::vector<const char *> Renderer::vGetRequiredExtensions()
{
  u32 glfwExtensionCount = 0;
  const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

  if (mEnableValidationLayers)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::vDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData)
{
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    Logger::Error("Validation Layer: {}", pCallbackData->pMessage);
  else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    Logger::Warning("Validation Layer: {}", pCallbackData->pMessage);
  else
    Logger::Info("Validation Layer: {}", pCallbackData->pMessage);

  return VK_FALSE;
}

void Renderer::vGetExtensions()
{
  u32 extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  DynamicArray<VkExtensionProperties> extensions(extensionCount);

  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.Data());

  Logger::Info("Available Extensions");

  for (const VkExtensionProperties &extension : extensions)
  {
    Logger::Message("{}", extension.extensionName);
  }
}

void Renderer::vSetValidationLayers()
{
  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif
}

bool Renderer::vCheckValidationLayerSupport()
{
  u32 layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  DynamicArray<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.Data());

  Logger::Info("Available Validation Layers");

  for (const VkLayerProperties &layer : availableLayers)
  {
    Logger::Message("{}", layer.layerName);
  }

  for (const char *layerName : mValidationLayers)
  {
    bool layerFound = false;

    for (const VkLayerProperties &layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }

    if (!layerFound)
    {
      Logger::Error("Validation Layer {} not found", layerName);
      return false;
    }
  }

  return true;
}

void Renderer::vCreateSurface()
{
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(mInstance, mWindow->GetGlfwWindow(), nullptr, &surface) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface!");
  }

  mSurface = surface;
}

void Renderer::vSetupDebugMessenger()
{
  if (!mEnableValidationLayers)
    return;

  vk::DispatchLoaderDynamic dispatchLoader(mInstance, vkGetInstanceProcAddr);

  vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
  createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  createInfo.pfnUserCallback = vDebugCallback;

  VK_TRY(mInstance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &mDebugMessenger, dispatchLoader),
         "Failed to create Debug Messenger!");
}

void Renderer::vCreateLogicalDevice()
{
  ru::QueueFamilyIndices indices = ru::vFindQueueFamilies(mPhysicalDevice, mSurface);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

  std::set<u32> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  f32 queuePriority = 1.0f;

  for (u32 queueFamily : uniqueQueueFamilies)
  {
    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.setWideLines(vk::True);
  vk::DeviceCreateInfo createInfo{};
  createInfo.queueCreateInfoCount = queueCreateInfos.size();
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  const std::vector<const char *> &deviceExtensions = ru::vGetDeviceExtensions();

  createInfo.enabledExtensionCount = (u32)deviceExtensions.size();
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (mEnableValidationLayers)
  {
    createInfo.enabledLayerCount = (u32)mValidationLayers.size();
    createInfo.ppEnabledLayerNames = mValidationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  VK_TRY(mPhysicalDevice.createDevice(&createInfo, nullptr, &mDevice), "Failed to create logical device!");

  mGraphicsQueue = mDevice.getQueue(indices.graphicsFamily.value(), 0);
  mPresentQueue = mDevice.getQueue(indices.presentFamily.value(), 0);
}

void Renderer::vCreateSwapchain()
{
  ru::SwapChainSupportDetails swapChainSupport = ru::vQuerySwapChainSupport(*this);

  vk::SurfaceFormatKHR surfaceFormat = ru::vChooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode = ru::vChooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = ru::vChooseSwapExtent(*this, swapChainSupport.capabilities);

  u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo{};
  createInfo.surface = mSurface;

  mSwapchainImageFormat = surfaceFormat.format;
  mSwapchainExtent = extent;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

  ru::QueueFamilyIndices indices = ru::vFindQueueFamilies(mPhysicalDevice, mSurface);

  u32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode = presentMode;
  createInfo.clipped = vk::True;
  createInfo.oldSwapchain = nullptr;

  mSwapchain = mDevice.createSwapchainKHR(createInfo);

  mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);
}

void Renderer::vCreateImageViews()
{
  mSwapchainImageViews.resize(mSwapchainImages.size());

  for (size_t i = 0; i < mSwapchainImages.size(); i++)
  {
    vk::ImageViewCreateInfo createInfo;
    createInfo.image = mSwapchainImages[i];
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = mSwapchainImageFormat;
    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    mSwapchainImageViews[i] = mDevice.createImageView(createInfo);
  }
}

void Renderer::vCreateGraphicsPipeline()
{
  vk::ShaderModule vertModule = ru::CreateShaderModule(mDevice, "base.vert");
  vk::ShaderModule fragModule = ru::CreateShaderModule(mDevice, "base.frag");

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
  vertShaderStageInfo.setModule(vertModule);
  vertShaderStageInfo.setPName("main");

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
  fragShaderStageInfo.setModule(fragModule);
  fragShaderStageInfo.setPName("main");

  vk::PipelineShaderStageCreateInfo shaderStages[] = {
      vertShaderStageInfo,
      fragShaderStageInfo,
  };

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();

  vertexInputInfo.setVertexBindingDescriptions(bindingDescription);
  vertexInputInfo.setVertexAttributeDescriptions(attributeDescriptions);

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.setTopology(vk::PrimitiveTopology::eLineStrip);
  inputAssembly.setPrimitiveRestartEnable(vk::True);

  vk::Viewport viewport{};
  viewport.setX(0.0f);
  viewport.setY(0.0f);
  viewport.setWidth((f32)mSwapchainExtent.width);
  viewport.setHeight((f32)mSwapchainExtent.height);
  viewport.setMinDepth(0.0f);
  viewport.setMaxDepth(1.0f);

  vk::Rect2D scissor{};
  scissor.setOffset({0, 0});
  scissor.setExtent(mSwapchainExtent);

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor,
                                                 vk::DynamicState::eLineWidth};

  vk::PipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.setDynamicStates(dynamicStates);

  vk::PipelineViewportStateCreateInfo viewportState{};
  viewportState.setViewports(viewport);
  viewportState.setScissors(scissor);

  vk::PipelineRasterizationStateCreateInfo rasterizer;
  rasterizer.setDepthClampEnable(vk::False);
  rasterizer.setRasterizerDiscardEnable(vk::False);
  rasterizer.setPolygonMode(vk::PolygonMode::eFill);
  rasterizer.setLineWidth(2.0f);
  rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
  rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);

  rasterizer.setDepthBiasEnable(vk::False);
  rasterizer.setDepthBiasConstantFactor(0.0f);
  rasterizer.setDepthBiasClamp(0.0f);
  rasterizer.setDepthBiasSlopeFactor(0.0f);

  vk::PipelineMultisampleStateCreateInfo multisampling{};
  multisampling.setSampleShadingEnable(vk::False);
  multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
  multisampling.setMinSampleShading(1.0f);
  multisampling.setPSampleMask(nullptr);
  multisampling.setAlphaToCoverageEnable(vk::False);
  multisampling.setAlphaToOneEnable(vk::False);

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                         vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  colorBlendAttachment.setBlendEnable(vk::False);
  colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
  colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
  /*colorBlendAttachment.setBlendEnable(vk::True);*/
  /*colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);*/
  /*colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);*/
  colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
  colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
  colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
  colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

  vk::PipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.setLogicOpEnable(vk::False);
  colorBlending.setLogicOp(vk::LogicOp::eCopy);
  colorBlending.setAttachments(colorBlendAttachment);
  colorBlending.setBlendConstants({0.f, 0.f, 0.f, 0.f});

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.setSetLayouts(mDescriptorSetLayout);
  pipelineLayoutInfo.setPushConstantRanges(nullptr);

  mPipelineLayout = mDevice.createPipelineLayout(pipelineLayoutInfo);
  if (!mPipelineLayout)
  {
    throw std::runtime_error("Failed to create pipeline layout");
  }

  vk::GraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.setStages(shaderStages)
      .setPVertexInputState(&vertexInputInfo)
      .setPInputAssemblyState(&inputAssembly)
      .setPViewportState(&viewportState)
      .setPMultisampleState(&multisampling)
      .setPDepthStencilState(nullptr)
      .setPColorBlendState(&colorBlending)
      .setPRasterizationState(&rasterizer)
      .setPDynamicState(&dynamicState)
      .setLayout(mPipelineLayout)
      .setRenderPass(mRenderPass)
      .setSubpass(0)
      .setBasePipelineHandle(VK_NULL_HANDLE)
      .setBasePipelineIndex(-1);

  auto result = mDevice.createGraphicsPipeline({}, pipelineInfo);

  if (result.result != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create graphics pipelines!");
  }
  mGraphicsPipeline = result.value;

  mDevice.destroyShaderModule(vertModule);
  mDevice.destroyShaderModule(fragModule);
}

void Renderer::vCreateRenderPass()
{
  vk::AttachmentDescription colorAttachment{};
  colorAttachment.setFormat(mSwapchainImageFormat);
  colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
  colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
  colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
  colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
  colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
  colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference colorAttachmentRef{};
  colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

  vk::SubpassDescription subpass{};
  subpass.setColorAttachments(colorAttachmentRef);

  vk::RenderPassCreateInfo renderPassInfo{};
  renderPassInfo.setAttachments(colorAttachment);
  renderPassInfo.setSubpasses(subpass);

  vk::SubpassDependency dependency;
  dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
      .setDstSubpass(0)
      .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
      .setSrcAccessMask(vk::AccessFlags(0))
      .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
      .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

  renderPassInfo.setDependencies(dependency);

  mRenderPass = mDevice.createRenderPass(renderPassInfo);
}

void Renderer::vCreateDescriptorSetLayout()
{
  vk::DescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.setBinding(0);
  uboLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
  uboLayoutBinding.setDescriptorCount(1);
  uboLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
  uboLayoutBinding.setPImmutableSamplers(nullptr);

  vk::DescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.setBindings(uboLayoutBinding);

  mDescriptorSetLayout = mDevice.createDescriptorSetLayout(layoutInfo);
  if (!mDescriptorSetLayout)
  {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }
}

void Renderer::vCreateFrameBuffers()
{
  mSwapChainFrameBuffers.resize(mSwapchainImageViews.size());

  for (size_t i = 0; i < mSwapchainImageViews.size(); i++)
  {
    VkImageView attachments[] = {mSwapchainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = mRenderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = mSwapchainExtent.width;
    framebufferInfo.height = mSwapchainExtent.height;
    framebufferInfo.layers = 1;

    mSwapChainFrameBuffers[i] = mDevice.createFramebuffer(framebufferInfo);
  }
}

void Renderer::vCreateCommandPool()
{
  ru::QueueFamilyIndices queueFamilyIndices = ru::vFindQueueFamilies(mPhysicalDevice, mSurface);

  vk::CommandPoolCreateInfo poolInfo;
  poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
      .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

  mCommandPool = mDevice.createCommandPool(poolInfo);
}

void Renderer::vCreateVertexBuffer()
{
  vk::DeviceSize bufferSize = sizeof(Vertices[0]) * Vertices.size();

  vk::Buffer stagingBuffer;
  vk::DeviceMemory stagingBufferMemory;
  ru::vCreateBuffer(mDevice, mPhysicalDevice, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
                    stagingBufferMemory);

  void *data;
  VK_TRY(mDevice.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(0), &data), "Failed to map memory");
  memcpy(data, Vertices.data(), bufferSize);
  mDevice.unmapMemory(stagingBufferMemory);

  ru::vCreateBuffer(mDevice, mPhysicalDevice, bufferSize,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::MemoryPropertyFlagBits::eDeviceLocal, mVertexBuffer, mVertexBufferMemory);
  ru::vCopyBuffer(mDevice, mCommandPool, mGraphicsQueue, stagingBuffer, mVertexBuffer, bufferSize);

  mDevice.destroyBuffer(stagingBuffer);
  mDevice.freeMemory(stagingBufferMemory);
}

void Renderer::vCreateIndexBuffer()
{
  vk::DeviceSize bufferSize = sizeof(Indices[0]) * Indices.size();

  vk::Buffer stagingBuffer;
  vk::DeviceMemory stagingBufferMemory;
  ru::vCreateBuffer(mDevice, mPhysicalDevice, bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
                    stagingBufferMemory);

  void *data;
  VK_TRY(mDevice.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(0), &data), "Failed to map memory");
  memcpy(data, Indices.data(), bufferSize);
  mDevice.unmapMemory(stagingBufferMemory);

  ru::vCreateBuffer(mDevice, mPhysicalDevice, bufferSize,
                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                    vk::MemoryPropertyFlagBits::eDeviceLocal, mIndexBuffer, mIndexBufferMemory);
  ru::vCopyBuffer(mDevice, mCommandPool, mGraphicsQueue, stagingBuffer, mIndexBuffer, bufferSize);

  mDevice.destroyBuffer(stagingBuffer);
  mDevice.freeMemory(stagingBufferMemory);
}

void Renderer::vCreateUniformBuffers()
{
  vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

  mUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  mUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  mUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    ru::vCreateBuffer(mDevice, mPhysicalDevice, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                      mUniformBuffers[i], mUniformBuffersMemory[i]);
    mUniformBuffersMapped[i] = mDevice.mapMemory(mUniformBuffersMemory[i], 0, bufferSize, vk::MemoryMapFlags(0));
  }
}

void Renderer::vCreateDescriptorPool()
{
  std::array<vk::DescriptorPoolSize, 1> poolSizes{};
  poolSizes[0].setType(vk::DescriptorType::eUniformBuffer);
  poolSizes[0].setDescriptorCount(MAX_FRAMES_IN_FLIGHT);

  vk::DescriptorPoolCreateInfo poolInfo{};
  poolInfo.setPoolSizes(poolSizes);
  poolInfo.setMaxSets(MAX_FRAMES_IN_FLIGHT);

  mDescriptorPool = mDevice.createDescriptorPool(poolInfo);
}

void Renderer::vCreateDescriptorSets()
{
  std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);

  vk::DescriptorSetAllocateInfo allocInfo{};
  allocInfo.setDescriptorPool(mDescriptorPool);
  allocInfo.setSetLayouts(layouts);

  mDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  mDescriptorSets = mDevice.allocateDescriptorSets(allocInfo);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.setBuffer(mUniformBuffers[i]);
    bufferInfo.setOffset(0);
    bufferInfo.setRange(sizeof(UniformBufferObject));

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.setDstSet(mDescriptorSets[i]);
    descriptorWrite.setDstBinding(0);
    descriptorWrite.setDstArrayElement(0);
    descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    descriptorWrite.setDescriptorCount(1);
    descriptorWrite.setPBufferInfo(&bufferInfo);

    mDevice.updateDescriptorSets(descriptorWrite, nullptr);
  }
}

void Renderer::vCreateCommandBuffers()
{
  mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.setCommandPool(mCommandPool)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandBufferCount((u32)mCommandBuffers.size());
  mCommandBuffers = mDevice.allocateCommandBuffers(allocInfo);
}

void Renderer::vRecordCommandBuffer(const vk::CommandBuffer &commandBuffer, u32 imageIndex)
{
  vk::CommandBufferBeginInfo beginInfo;

  commandBuffer.begin(beginInfo);

  vk::RenderPassBeginInfo renderPassInfo;
  renderPassInfo.setRenderPass(mRenderPass)
      .setFramebuffer(mSwapChainFrameBuffers[imageIndex])
      .setRenderArea(vk::Rect2D({0, 0}, mSwapchainExtent));

  vk::ClearValue clearColor({0.f, 0.f, 0.f, 1.f});
  renderPassInfo.setClearValues(clearColor);

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);
  commandBuffer.setViewport(0, vk::Viewport()
                                   .setX(0.f)
                                   .setY(0.f)
                                   .setWidth((f32)mSwapchainExtent.width)
                                   .setHeight((f32)mSwapchainExtent.height)
                                   .setMinDepth(0.f)
                                   .setMaxDepth(1.f));
  commandBuffer.setLineWidth(2.0f);
  commandBuffer.setScissor(0, vk::Rect2D().setOffset({0, 0}).setExtent(mSwapchainExtent));

  vk::Buffer buffers[] = {mVertexBuffer};
  vk::DeviceSize offsets[] = {0};

  vUpdateUniformBuffer(mCurrentFrame);

  commandBuffer.bindVertexBuffers(0, 1, buffers, offsets);
  commandBuffer.bindIndexBuffer(mIndexBuffer, 0, vk::IndexType::eUint16);
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, mDescriptorSets[mCurrentFrame],
                                   nullptr);

  commandBuffer.drawIndexed(static_cast<u32>(Indices.size()), 1, 0, 0, 0);
  commandBuffer.endRenderPass();
  commandBuffer.end();
}

void Renderer::vUpdateUniformBuffer(u32 currentImage)
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  f32 time = std::chrono::duration<f32, std::chrono::seconds::period>(currentTime - startTime).count();

  UniformBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
  ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
  ubo.proj = glm::perspective(glm::radians(45.f), mSwapchainExtent.width / (f32)mSwapchainExtent.height, 0.1f, 10.f);
  ubo.proj[1][1] *= -1;

  memcpy(mUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Renderer::vCreateSyncObjects()
{
  mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  vk::SemaphoreCreateInfo semaphoreInfo;
  vk::FenceCreateInfo fenceCreateInfo;
  fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    mImageAvailableSemaphores[i] = mDevice.createSemaphore(semaphoreInfo);
    mRenderFinishedSemaphores[i] = mDevice.createSemaphore(semaphoreInfo);
    mInFlightFences[i] = mDevice.createFence(fenceCreateInfo);
  }
}

void Renderer::vRecreateSwapchain()
{
  i32 width = 0, height = 0;
  while (width == 0 || height == 0)
  {
    glfwGetFramebufferSize(mWindow->GetGlfwWindow(), &width, &height);
    glfwWaitEvents();
  }

  mDevice.waitIdle();

  vCleanupSwapchain();

  vCreateSwapchain();
  vCreateImageViews();
  vCreateFrameBuffers();
}

void Renderer::vCleanupSwapchain()
{
  for (size_t i = 0; i < mSwapChainFrameBuffers.size(); i++)
  {
    mDevice.destroyFramebuffer(mSwapChainFrameBuffers[i]);
  }

  for (size_t i = 0; i < mSwapchainImageViews.size(); i++)
  {
    mDevice.destroyImageView(mSwapchainImageViews[i]);
  }

  mDevice.destroySwapchainKHR(mSwapchain);
}

void Renderer::DrawFrame()
{
  vk::Result result = mDevice.waitForFences(mInFlightFences[mCurrentFrame], vk::True, UINT64_MAX);

  vk::ResultValue resultVal =
      mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame]);

  if (resultVal.result == vk::Result::eErrorOutOfDateKHR)
  {
    vRecreateSwapchain();
    return;
  }
  else if (resultVal.result != vk::Result::eSuccess && resultVal.result != vk::Result::eSuboptimalKHR)
  {
    Logger::Error("Failed to acquire swap chain image!");
  }

  mDevice.resetFences(mInFlightFences[mCurrentFrame]);

  u32 imageIndex = resultVal.value;

  mCommandBuffers[mCurrentFrame].reset(vk::CommandBufferResetFlags(0));

  vRecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

  vk::SubmitInfo submitInfo;
  vk::PipelineStageFlags waitStages[]{vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.setWaitSemaphores(mImageAvailableSemaphores[mCurrentFrame])
      .setWaitDstStageMask(waitStages)
      .setCommandBuffers(mCommandBuffers[mCurrentFrame])
      .setSignalSemaphores(mRenderFinishedSemaphores[mCurrentFrame]);

  mGraphicsQueue.submit(submitInfo, mInFlightFences[mCurrentFrame]);

  vk::PresentInfoKHR presentInfo;
  presentInfo.setWaitSemaphores(mRenderFinishedSemaphores[mCurrentFrame]);
  presentInfo.setSwapchains(mSwapchain).setImageIndices(imageIndex);

  try
  {
    result = mPresentQueue.presentKHR(presentInfo);
  }
  catch (const vk::OutOfDateKHRError &e)
  {
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR ||
        mWindow->GetFramebufferResized())
    {
      vRecreateSwapchain();
      mWindow->SetFramebufferResized(false);
    }
    else if (result != vk::Result::eSuccess)
    {
      Logger::Error("failed to present swap chain image!");
    }
  }

  mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::Clean()
{
  CHECK_IN();

  mDevice.waitIdle();

  vCleanupSwapchain();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    mDevice.destroyBuffer(mUniformBuffers[i]);
    mDevice.freeMemory(mUniformBuffersMemory[i]);
  }

  mDevice.destroyDescriptorPool(mDescriptorPool);
  mDevice.destroyDescriptorSetLayout(mDescriptorSetLayout);

  mDevice.destroyPipeline(mGraphicsPipeline);
  mDevice.destroyPipelineLayout(mPipelineLayout);
  mDevice.destroyRenderPass(mRenderPass);

  mDevice.destroyBuffer(mIndexBuffer);
  mDevice.freeMemory(mIndexBufferMemory);

  mDevice.destroyBuffer(mVertexBuffer);
  mDevice.freeMemory(mVertexBufferMemory);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    mDevice.destroySemaphore(mRenderFinishedSemaphores[i]);
    mDevice.destroySemaphore(mImageAvailableSemaphores[i]);
    mDevice.destroyFence(mInFlightFences[i]);
  }

  mDevice.destroyCommandPool(mCommandPool);
  mDevice.destroy();

  if (mEnableValidationLayers)
  {
    vk::DispatchLoaderDynamic dispatchLoader(mInstance, vkGetInstanceProcAddr);
    mInstance.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr, dispatchLoader);
  }

  mInstance.destroySurfaceKHR(mSurface);
  mInstance.destroy();
}

} // namespace jet
