#ifndef TECK_ENGINE_H
#define TECK_ENGINE_H

#include "core.h"
#include "renderer.h"
#include "window.h"
#include <vector>

namespace tk
{

class Engine
{
  u8 bRunning : 1;
  u8 bIsServer : 1;

private:
  Engine();

private:
  Renderer mRenderer;
  Window mWindow;

  std::vector<class SUpdate *> mUpdateSystems;

private:
  void InitSystems();
  void CleanSystems();

public:
  static i32 Run(i32 argc, char **argv);

private:
  void Init();

  void ParseArgs(i32 argc, char **argv);

  void PollEvents();
  void Loop();

  void Clean();
};

} // namespace tk

#endif // TECK_ENGINE_H
