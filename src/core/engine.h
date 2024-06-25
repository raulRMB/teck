#ifndef JET_ENGINE_H
#define JET_ENGINE_H

#include "core.h"
#include "networking/nettest.h"
#include "renderer.h"
#include "window.h"
#include <vector>

namespace jet
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

  Server mServer;
  Client mClient;

  std::vector<class SUpdate *> mUpdateSystems;

private:
  void InitSystems();
  void CleanSystems();

public:
  static i32 Run(i32 argc, char **argv);

private:
  void Init();

  void ParseArgs(i32 argc, char **argv);

  void NetPoll();
  void PollEvents();
  void Loop();

  void Clean();
};

} // namespace jet

#endif // JET_ENGINE_H
