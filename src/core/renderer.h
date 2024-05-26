#ifndef JET_RENDERER_H
#define JET_RENDERER_H

#include <vulkan/vulkan.hpp>

namespace jet
{

class Renderer
{
  vk::Instance mInstance;

  friend class Engine;

private:
  void vCreateInstance();
};

}

#endif //JET_RENDERER_H
