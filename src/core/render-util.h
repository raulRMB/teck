#include "def.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace jet
{
class Renderer;
}

namespace jet::ru
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

struct SwapChainSupportDetails
{
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

SwapChainSupportDetails vQuerySwapChainSupport(const Renderer &renderer);

bool vIsDeviceSuitable(const Renderer &renderer);
void vPickPhysicalDevice(vk::Instance &instance, vk::PhysicalDevice &physicalDevice);
i32 vRateDeviceSuitability(const vk::PhysicalDevice &device);

QueueFamilyIndices vFindQueueFamilies(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);

std::vector<const char *> vGetDeviceExtensions();
bool vCheckDeviceExtensionSupport(const Renderer &renderer);

vk::SurfaceFormatKHR vChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
vk::PresentModeKHR vChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
vk::Extent2D vChooseSwapExtent(const Renderer &renderer, const vk::SurfaceCapabilitiesKHR &capabilities);

} // namespace jet::ru
