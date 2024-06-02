#include "render-util.h"
#include "logger.h"
#include <map>

namespace jet::RendererUtil
{

bool vIsDeviceSuitable(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
{
  vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
  vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

  QueueFamilyIndices indices = vFindQueueFamilies(device, surface);
  if (!indices.isComplete())
  {
    return false;
  }

  return deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader;
}

void vPickPhysicalDevice(vk::Instance &instance, vk::PhysicalDevice &physicalDevice)
{
  std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

  if (devices.size() == 0)
  {
    throw std::runtime_error("Failed to find GPUs with Vulkan support...Time to upgrade your hardware! :D");
  }

  std::multimap<i32, vk::PhysicalDevice> candidates;

  for (const vk::PhysicalDevice &device : devices)
  {
    i32 score = vRateDeviceSuitability(device);
    candidates.insert(std::make_pair(score, device));
  }

  if (candidates.rbegin()->first > 0)
  {
    physicalDevice = candidates.rbegin()->second;
    Logger::Info("Selected GPU: {}", physicalDevice.getProperties().deviceName.data());
  }
  else
  {
    throw std::runtime_error("Failed to find a suitable GPU!");
  }
}

i32 vRateDeviceSuitability(const vk::PhysicalDevice &device)
{
  vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
  vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

  i32 score = 0;

  if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
  {
    score += 1000;
  }

  score += deviceProperties.limits.maxImageDimension2D;

  if (!deviceFeatures.geometryShader)
  {
    return 0;
  }

  return score;
}

QueueFamilyIndices vFindQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
{
  QueueFamilyIndices indices;

  std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

  u32 i = 0;
  for (const vk::QueueFamilyProperties &queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
    {
      indices.graphicsFamily = i;
    }

    if (device.getSurfaceSupportKHR(i, surface))
    {
      indices.presentFamily = i;
    }

    if (indices.isComplete())
    {
      break;
    }

    i++;
  }

  return indices;
}

} // namespace jet::RendererUtil
