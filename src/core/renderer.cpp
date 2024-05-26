#include "renderer.h"

#include "core.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include "dynamic-array.h"

#include "render-util.h"

namespace jet
{

void Renderer::vCreateInstance()
{
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

  u32 glfwExtensionCount = 0;
  const char** glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  dArray<const char*> requiredExtensions;

  for (u32 i = 0; i < glfwExtensionCount; i++)
  {
    requiredExtensions.PushBack(glfwExtensions[i]);
  }
  
  requiredExtensions.PushBack(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  
  createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

  createInfo.enabledExtensionCount = (u32)requiredExtensions.Size();
  createInfo.ppEnabledExtensionNames = requiredExtensions.Data();

  createInfo.enabledLayerCount = 0;

  TRY_VK(vk::createInstance(&createInfo, nullptr, &mInstance), "Failed to create VkInstance");

  u32 extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  dArray<VkExtensionProperties> extensions(extensionCount);

  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.Data());

  Logger::Info("Avaliable Extensions:\n");
}

} // namespace jet
