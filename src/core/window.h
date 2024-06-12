#ifndef JET_WINDOW_H
#define JET_WINDOW_H

#include <GLFW/glfw3.h>

#include <string>

#include "core.h"

namespace jet
{

class Window
{
  i32 mWidth;
  i32 mHeight;
  std::string mName;

  GLFWwindow *pGlfwWindow{};

  u8 mFramebufferResized : 1;

public:
  Window(i32 Width = 800, i32 Height = 600, std::string &&Name = "jet");

private:
  friend class Engine;

  void Init();

  bool ShouldClose();
  void PollEvents();

  void SetGLFWCallbacks();

  static void FramebufferResizeCallback(GLFWwindow *window, i32 width, i32 height);

  void Clean();

public:
  GLFWwindow *GetGlfwWindow() const;
  void SetFramebufferResized(bool value);
  bool GetFramebufferResized() const;

  u32 GetWidth() const;
  u32 GetHeight() const;
};

} // namespace jet

#endif // JET_WINDOW_H
