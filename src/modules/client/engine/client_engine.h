#ifndef TK_CLIENT_ENGINE_H
#define TK_CLIENT_ENGINE_H

#include "core/engine.h"

namespace tk
{

class ClientEngine : public Engine
{
  class Renderer* mRenderer{};
  class Window* mWindow{};

public:
  ClientEngine();

  static i32 Run(i32 argc, char** arcv);

  virtual void Init() override;
  virtual void Draw() override;
  virtual void Clean() override;
  virtual void PollEvents() override;
};

} // namespace tk

#endif // TK_CLIENT_ENGINE_H
