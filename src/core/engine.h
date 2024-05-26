#ifndef JET_ENGINE_H
#define JET_ENGINE_H

#include "renderer.h"
#include "core.h"
#include "window.h"

namespace jet
{

class Engine
{
  u8 bRunning : 1;

private:
  Engine();
  
private: 
  Renderer mRenderer;
  Window mWindow;

public:
  static i32 Run();

private:
  void Init();

  void PollEvents();
  void Loop();

  void Clean();
};
  
}

#endif//JET_ENGINE_H
