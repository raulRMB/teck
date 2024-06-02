#include "def.h"
#include <vulkan/vulkan.hpp>

namespace jet::RendererUtil
{

#define VK_TRY(fn, msg)                                                                                                \
  if (vk::Result result = fn; result != vk::Result::eSuccess)                                                          \
  {                                                                                                                    \
    throw std::runtime_error(msg);                                                                                     \
  }

bool vIsDeviceSuitable(const vk::PhysicalDevice &device);
void vPickPhysicalDevice(vk::Instance &instance, vk::PhysicalDevice &physicalDevice);
i32 vRateDeviceSuitability(const vk::PhysicalDevice &device);

u32 vFindQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);

} // namespace jet::RendererUtil
