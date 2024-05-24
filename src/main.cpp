#include "vulkan/vulkan_core.h"
#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int main()
{
  glfwInit();

  GLFWwindow* window = glfwCreateWindow(400, 400, "jet", nullptr, nullptr);

  uint32_t ext_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

  while(!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
  }

  glfwTerminate();
}
