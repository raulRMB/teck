#ifndef JET_RENDERER_H
#define JET_RENDERER_H

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace jet
{

class Renderer
{
  const std::vector<const char *> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
  static constexpr bool mEnableValidationLayers = false;
#else
  static constexpr bool mEnableValidationLayers = true;
#endif

  vk::Instance mInstance;
  vk::DebugUtilsMessengerEXT mDebugMessenger;
  vk::PhysicalDevice mPhysicalDevice;
  vk::Device mDevice;
  vk::Queue mGraphicsQueue;
  vk::Queue mPresentQueue;
  vk::SurfaceKHR mSurface;
  vk::SwapchainKHR mSwapchain;
  std::vector<vk::Image> mSwapchainImages;
  std::vector<vk::ImageView> mSwapchainImageViews;
  vk::Format mSwapchainImageFormat;
  vk::Extent2D mSwapchainExtent;
  vk::Pipeline mGraphicsPipeline;

  class Window *mWindow;

  friend class Engine;

public:
  const vk::PhysicalDevice &GetPhysicalDevice() const;
  const vk::Device &GetDevice() const;
  const vk::SurfaceKHR &GetSurfaceKHR() const;
  const Window &GetWindow() const;

private:
  void Init(class Window *window);

  void vCreateInstance();
  void vGetExtensions();
  void vSetValidationLayers();
  bool vCheckValidationLayerSupport();
  std::vector<const char *> vGetRequiredExtensions();
  static VKAPI_ATTR VkBool32 VKAPI_CALL vDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                       void *pUserData);

  void vSetupDebugMessenger();
  void vCreateSurface();
  void vCreateLogicalDevice();
  void vCreateSwapchain();
  void vCreateImageViews();
  void vCreateGraphicsPipeline();

  void Clean();
};

} // namespace jet

#endif // JET_RENDERER_H
