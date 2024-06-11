#include "window.h"
#include "logger.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace jet
{

Window::Window(i32 Width, i32 Height, std::string &&Name)
    : mWidth(Width), mHeight(Height), mFramebufferResized(false), mName(std::move(Name))
{
}

GLFWwindow *Window::GetGlfwWindow() const
{
  return pGlfwWindow;
}

void Window::Init()
{
  CHECK_IN();

  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  /*glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);*/
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  pGlfwWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(pGlfwWindow, this);

  SetGLFWCallbacks();
  if (!pGlfwWindow)
  {
    throw std::runtime_error("Failed to create GLFWwindow");
  }
}

void Window::SetGLFWCallbacks()
{
  glfwSetFramebufferSizeCallback(pGlfwWindow, FramebufferResizeCallback);
}

bool Window::ShouldClose()
{
  return glfwWindowShouldClose(pGlfwWindow);
}

void Window::PollEvents()
{
  glfwPollEvents();
}

void Window::FramebufferResizeCallback(GLFWwindow *window, i32 width, i32 height)
{
  Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  pWindow->mFramebufferResized = true;
  pWindow->mWidth = width;
  pWindow->mHeight = height;
}

void Window::SetFramebufferResized(bool value)
{
  mFramebufferResized = value;
}

bool Window::GetFramebufferResized() const
{
  return mFramebufferResized;
}

void Window::Clean()
{
  CHECK_IN();

  glfwDestroyWindow(pGlfwWindow);
  glfwTerminate();
}

} // namespace jet
