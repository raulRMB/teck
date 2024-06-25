#include "types.h"
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

vk::ShaderModule CreateShaderModule(const vk::Device &device, const std::string &shaderName);

u32 vFindMemoryType(const vk::PhysicalDevice &physicalDevice, u32 typeFilter, vk::MemoryPropertyFlags properties);

void vCreateBuffer(const vk::Device &device, const vk::PhysicalDevice &physicalDevice, vk::DeviceSize size,
                   vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer,
                   vk::DeviceMemory &bufferMemory);

void vCopyBuffer(const vk::Device &device, const vk::CommandPool &commandPool, const vk::Queue &graphicsQueue,
                 const vk::Buffer &srcBuffer, const vk::Buffer &dstBuffer, vk::DeviceSize size);

} // namespace jet::ru
