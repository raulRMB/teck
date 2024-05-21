#include <iostream>
#include <GLFW/glfw3.h>

int main()
{
  glfwInit();

  GLFWwindow* window = glfwCreateWindow(400, 400, "jet", nullptr, nullptr);

  while(!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
  }

  glfwTerminate();
}
