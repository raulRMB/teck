#ifndef JET_RENDERER_H
#define JET_RENDERER_H

#include <vector>
#include <vulkan/vulkan.hpp>

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

  class Window *mWindow;

  vk::SurfaceKHR mSurface;

  vk::Queue mGraphicsQueue;
  vk::Queue mPresentQueue;

  friend class Engine;

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

  void Clean();
};

} // namespace jet

#endif // JET_RENDERER_H
