#ifndef JET_ENGINE_H
#define JET_ENGINE_H

#include "core.h"
#include "renderer.h"
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

} // namespace jet

#endif // JET_ENGINE_H
