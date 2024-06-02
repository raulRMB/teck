#include "renderer.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "core.h"
#include "dynamic-array.h"
#include "logger.h"
#include "render-util.h"

namespace jet
{

void Renderer::Init()
{
  CHECK_IN();

  vCreateInstance();
  vSetupDebugMessenger();
  RendererUtil::vPickPhysicalDevice(mInstance, mPhysicalDevice);
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

void Renderer::vSetupDebugMessenger()
{
  if (!mEnableValidationLayers)
    return;

  vk::DispatchLoaderDynamic dispatchLoader(mInstance, vkGetInstanceProcAddr);

  vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
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

void Renderer::Clean()
{
  CHECK_IN();

  vkDestroyInstance(mInstance, nullptr);
}

} // namespace jet
