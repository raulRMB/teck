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
  
  GLFWwindow* pGlfwWindow{};

public:
  Window(i32 Width = 800, i32 Height = 600, std::string&& Name = "jet");

private:
  friend class Engine;

  void Init();

  bool ShouldClose();
  void PollEvents();

  void Clean();

public:
  GLFWwindow* GetGlfwWindow();
};
  
}

#endif //JET_WINDOW_H
