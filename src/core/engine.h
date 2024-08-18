#ifndef TECK_ENGINE_H
#define TECK_ENGINE_H

#include "core.h"
#include <vector>

namespace tk
{

class Engine
{
protected:
  u8 bRunning : 1;
  static Engine mInstance;

protected:
  Engine();

public:
  static Engine& Get();

private:
  std::vector<class SUpdate*> mUpdateSystems{};

private:
  void InitSystems();
  void CleanSystems();

protected:
  virtual void Init();

  virtual void ParseArgs(i32 argc, char** argv);

  virtual void PollEvents();
  virtual void Loop();
  virtual void Draw();

  virtual void Clean();
};

} // namespace tk

#endif // TECK_ENGINE_H
