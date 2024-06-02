#include "def.h"
#include <vulkan/vulkan.hpp>

namespace jet::RendererUtil
{

#define VK_TRY(fn, msg)                                                                                                \
  if (vk::Result result = fn; result != vk::Result::eSuccess)                                                          \
  {                                                                                                                    \
    throw std::runtime_error(msg);                                                                                     \
  }

struct QueueFamilyIndices
{
  std::optional<u32> graphicsFamily;
  std::optional<u32> presentFamily;

  inline bool isComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

bool vIsDeviceSuitable(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);
void vPickPhysicalDevice(vk::Instance &instance, vk::PhysicalDevice &physicalDevice);
i32 vRateDeviceSuitability(const vk::PhysicalDevice &device);

QueueFamilyIndices vFindQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);

} // namespace jet::RendererUtil
