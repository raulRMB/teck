#include "renderer.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "core.h"
#include "dynamic-array.h"
#include "logger.h"
#include "render-util.h"
#include "window.h"

#include <set>

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
  vCreateGraphicsPipeline();
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

  Logger::Info("Avaliable Extensions");

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

  vk::DebugUtilsMessengerEXT debugMessenger;
  VK_TRY(mInstance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger, dispatchLoader),
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

  mSwapchain = mDevice.createSwapchainKHR(createInfo, nullptr);

  if (mSwapchain == nullptr)
  {
    throw std::runtime_error("failed to create swapchain");
  }

  mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);
}

void Renderer::vCreateImageViews()
{
  mSwapchainImageViews.resize(mSwapchainImages.size());

  for (size_t i = 0; i < mSwapchainImages.size(); i++)
  {
    vk::ImageViewCreateInfo createInfo{};
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
    mSwapchainImageViews[i] = mDevice.createImageView(createInfo, nullptr);
  }
}

void Renderer::vCreateGraphicsPipeline()
{
}

void Renderer::Clean()
{
  CHECK_IN();
}

} // namespace jet
